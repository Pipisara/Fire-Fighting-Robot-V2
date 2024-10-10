/*
  Fire-Fighting Robot and Remote Control with Camera

  THIS CODE FOR ESP32 Cam Module

  This code controls a fire-fighting robot equipped with a remote controller using nRF24L01 for wireless communication.
   The robot features servos for camera and nozzle movement, motors for mobility, and sensors for fire detection. 
   It includes LEDs for status indication and a water pump system to extinguish flames. 
   The robot can move forward, backward, and turn, all controlled remotely, while the water pump activates when fire is detected.

  Author: Pipisara Chandrabhanu
*/



#include "src/OV2640.h"  // Include camera library for OV2640 module
#include <WiFi.h>        // Include WiFi library for ESP32
#include <WebServer.h>   // Include web server library for serving web requests
#include <WiFiClient.h>  // Include WiFi client library for handling client connections

#define CAMERA_MODEL_AI_THINKER  // Define the camera model as AI Thinker

#include "camera_pins.h"  // Include the camera pin configuration for the AI Thinker model

// WiFi credentials
const char* ssid = "Your WiFi SSID";  // Replace with your WiFi SSID
const char* password = "Password";    // Replace with your WiFi password

#define FLASH_LED_PIN 4  // Define the GPIO pin for the camera's flash LED
#define LED_PIN 33       // Define the GPIO pin for the built-in red LED on the ESP32

OV2640 cam;              // Initialize the camera object
WebServer server(80);    // Create a web server object on port 80

// HTTP response headers for streaming JPEG images
const char HEADER[] = "HTTP/1.1 200 OK\r\n" \
                      "Access-Control-Allow-Origin: *\r\n" \
                      "Content-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
const char BOUNDARY[] = "\r\n--123456789000000000000987654321\r\n";
const char CTNTTYPE[] = "Content-Type: image/jpeg\r\nContent-Length: ";
const int hdrLen = strlen(HEADER);  // Calculate the length of the header
const int bdrLen = strlen(BOUNDARY);  // Calculate the length of the boundary
const int cntLen = strlen(CTNTTYPE);  // Calculate the length of the content type string

bool wifiConnected = false;  // Flag to track WiFi connection status
unsigned long previousMillis = 0;  // Timer variable for LED blinking
const long interval = 500;    // Interval for blinking LED (500ms)

// Handle streaming of JPEG images
void handle_jpg_stream(void)
{
  char buf[32];  // Buffer to store the size of the image
  int s;

  WiFiClient client = server.client();  // Get the connected client

  client.write(HEADER, hdrLen);  // Send the header to the client
  client.write(BOUNDARY, bdrLen);  // Send the boundary to the client

  while (true)
  {
    if (!client.connected()) break;  // Stop if the client disconnects
    cam.run();  // Capture an image from the camera
    s = cam.getSize();  // Get the size of the image
    client.write(CTNTTYPE, cntLen);  // Send the content type to the client
    sprintf(buf, "%d\r\n\r\n", s);  // Format the size of the image
    client.write(buf, strlen(buf));  // Send the image size to the client
    client.write((char *)cam.getfb(), s);  // Send the image data to the client
    client.write(BOUNDARY, bdrLen);  // Send the boundary to the client
  }
}

// HTTP response header for serving a single JPEG image
const char JHEADER[] = "HTTP/1.1 200 OK\r\n" \
                       "Content-disposition: inline; filename=capture.jpg\r\n" \
                       "Content-type: image/jpeg\r\n\r\n";
const int jhdLen = strlen(JHEADER);  // Calculate the length of the header

// Handle serving a single JPEG image
void handle_jpg(void)
{
  WiFiClient client = server.client();  // Get the connected client

  cam.run();  // Capture an image from the camera
  if (!client.connected()) return;  // Stop if the client disconnects

  client.write(JHEADER, jhdLen);  // Send the header to the client
  client.write((char *)cam.getfb(), cam.getSize());  // Send the image data to the client
}

// Turn on the camera flash
void handle_flash_on()
{
  digitalWrite(FLASH_LED_PIN, HIGH);  // Turn on the flash LED
  server.send(200, "text/plain", "Flash is ON");  // Send a response to the client
}

// Turn off the camera flash
void handle_flash_off()
{
  digitalWrite(FLASH_LED_PIN, LOW);  // Turn off the flash LED
  server.send(200, "text/plain", "Flash is OFF");  // Send a response to the client
}

// Handle requests for undefined routes
void handleNotFound()
{
  String message = "Server is running!\n\n";  // Message to send when an undefined route is accessed
  message += "URI: ";  // Add the requested URI to the message
  message += server.uri();
  message += "\nMethod: ";  // Add the HTTP method to the message
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";  // Add the number of arguments to the message
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);  // Send the message to the client
}

// Setup function to initialize the camera, WiFi, and web server
void setup()
{
  Serial.begin(115200);  // Start the serial communication at 115200 baud rate

  pinMode(FLASH_LED_PIN, OUTPUT);  // Set the flash LED pin as an output
  digitalWrite(FLASH_LED_PIN, LOW);  // Turn off the flash LED initially
  pinMode(LED_PIN, OUTPUT);  // Set the built-in LED pin as an output

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;  // Set XCLK frequency to 20 MHz
  config.pixel_format = PIXFORMAT_JPEG;  // Set pixel format to JPEG

  // Frame parameters
  config.frame_size = FRAMESIZE_VGA;  // Set frame size to VGA
  config.jpeg_quality = 3;  // Set JPEG quality (1 = highest, 63 = lowest)
  config.fb_count = 2;  // Use 2 frame buffers

  cam.init(config);  // Initialize the camera with the specified configuration

  // Initialize WiFi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  // Connect to the WiFi network using credentials
}

// Main loop function to handle WiFi connection and client requests
void loop()
{
  unsigned long currentMillis = millis();  // Get the current time

  // If not connected to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (currentMillis - previousMillis >= interval) {  // Check if interval has passed
      previousMillis = currentMillis;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Blink the built-in LED
    }
  } else if (!wifiConnected) {
    wifiConnected = true;  // Mark WiFi as connected
    digitalWrite(LED_PIN, HIGH);  // Turn on the built-in LED

    // Print connection details to the serial monitor
    Serial.println(F("WiFi connected"));
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
    Serial.print("Stream Link: http://");
    Serial.print(ip);
    Serial.println("/live");

    // Print flash control URLs to the serial monitor
    Serial.print("Flash ON URL: http://");
    Serial.print(ip);
    Serial.println("/on");

    Serial.print("Flash OFF URL: http://");
    Serial.print(ip);
    Serial.println("/off");

    // Define server routes
    server.on("/live", HTTP_GET, handle_jpg_stream);  // Route for live stream
    server.on("/jpg", HTTP_GET, handle_jpg);          // Route for single image capture
    server.on("/on", HTTP_GET, handle_flash_on);      // Route for turning flash on
    server.on("/off", HTTP_GET, handle_flash_off);    // Route for turning flash off
    server.onNotFound(handleNotFound);                // Route for undefined routes

    server.begin();  // Start the server
  }

  server.handleClient();  // Handle incoming client requests
}
