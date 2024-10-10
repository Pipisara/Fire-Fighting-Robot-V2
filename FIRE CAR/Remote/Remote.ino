
/*
  Remote Control for Fire-Fighting Robot

  THIS CODE FOR ARDUIONO NANO in REMOTE


  This code is designed for controlling a fire-fighting robot via an nRF24L01 wireless module. The remote has switches to control the robot's movements (forward, backward, left, right), servo motors for camera and nozzle movement, a pump activation switch, and an LED for status indication. It sends the corresponding control signals to the robot based on the input from the switches.

  Author: Pipisara Chandrabhanu
*/

#include <SPI.h>       // Include SPI library
#include <nRF24L01.h>  // Include nRF24L01 library
#include <RF24.h>      // Include RF24 library

RF24 radio(9, 10);                // Define RF24 object with CE, CSN pins
const byte address[6] = "00001";  // Address for the communication channel

#define FSwitch 6  // directions switches
#define BSwitch 7
#define RSwitch 5
#define LSwitch 4

#define Lservosw 0  //  servo switches
#define Rservosw 1

#define CamLservosw A4 //  servo switches
#define CamRservosw A5

#define Flash A1

#define pumpsw 8  //  for pump switch

#define LED A2   //  for LED


void setup() {
  radio.begin();                      // Initialize radio
  radio.openWritingPipe(address);     // Open writing pipe
  radio.openReadingPipe(1, address);  // Open reading pipe
  radio.setPALevel(RF24_PA_LOW);      // Set power level

  // Set pin modes
  pinMode(FSwitch, INPUT_PULLUP);
  pinMode(BSwitch, INPUT_PULLUP);
  pinMode(LSwitch, INPUT_PULLUP);
  pinMode(RSwitch, INPUT_PULLUP);
  pinMode(Lservosw, INPUT_PULLUP);
  pinMode(Rservosw, INPUT_PULLUP);
  pinMode(pumpsw, INPUT_PULLUP);
  pinMode(CamLservosw, INPUT_PULLUP);
  pinMode(CamRservosw, INPUT_PULLUP);
  pinMode(Flash, INPUT_PULLUP);


  Serial.begin(9600);  // Initialize serial communication
}

void loop() {
  // Read state of switches
  int Forward = !digitalRead(FSwitch);
  int Backward = !digitalRead(BSwitch);
  int Left = !digitalRead(LSwitch);
  int Right = !digitalRead(RSwitch);
  int LServo = !digitalRead(Lservosw);
  int Rservo = !digitalRead(Rservosw);
  int pump = !digitalRead(pumpsw);
  int camL = !digitalRead(CamLservosw);
  int camR = !digitalRead(CamRservosw);
  int Flasher = !digitalRead(Flash);
  // Count how many switches are pressed
  int buttonCount = Forward + Backward + Left + Right + LServo + Rservo + pump + camL + camR + Flasher;

  // Check if both servo switches are pressed
  if (LServo == HIGH && Rservo == HIGH) {
    char dataToSend = 'Y';
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
    return;  //
  }
  if (camL == HIGH && camR == HIGH) {
    char dataToSend = 'K';
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
    return;  //
  }
  
  // Check if more than one switch is pressed
  else if (buttonCount >= 2) {
    char dataToSend = 'S';
    analogWrite(LED, 0);
    radio.write(&dataToSend, sizeof(dataToSend));
    delay(100);  // Delay for stability
    return;
  }

  // Check which direction switch is pressed and send corresponding signal
  if (Forward == HIGH) {
    char dataToSend = 'F';  // move forward
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
    
  } 
  else if (camL == HIGH) {
    char dataToSend = 'M';  // cam servo turn
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (camR == HIGH) {
    char dataToSend = 'N';  // cam servo turn
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  }
  else if (Backward == HIGH) {
    char dataToSend = 'B';  // move backward
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (Left == HIGH) {
    char dataToSend = 'L';  // turn left
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (Right == HIGH) {
    char dataToSend = 'R';  // turn right
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (LServo == HIGH) {
    char dataToSend = 'A';  // move left servo
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (Rservo == HIGH) {
    char dataToSend = 'D';  // move right servo
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } 
  else if (Flasher == HIGH) {
    char dataToSend = 'Z';  // move right servo
    delay(500);
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } 
  
  else if (pump == HIGH) {
    char dataToSend = 'W';  // operate pump
    delay(500);             // Delay for stability
    analogWrite(LED, 255);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else if (Right == LOW && Right == LOW && Right == LOW && Right == LOW) {
    char dataToSend = 'S';
    analogWrite(LED, 0);
    radio.write(&dataToSend, sizeof(dataToSend));
  } else {
    char dataToSend = 'S';
    analogWrite(LED, 0);
    radio.write(&dataToSend, sizeof(dataToSend));
  }
  
}
