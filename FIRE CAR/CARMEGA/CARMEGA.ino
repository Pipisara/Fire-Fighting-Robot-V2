/*
 Fire-Fighting Robot and Remote Control with Camera       

  THIS CODE FOR ARDUIONO MEGA ROBOT

  This code controls a fire-fighting robot equipped with a remote controller using nRF24L01 for wireless communication.
   The robot features servos for camera and nozzle movement, motors for mobility, and sensors for fire detection. 
   It includes LEDs for status indication and a water pump system to extinguish flames. 
   The robot can move forward, backward, and turn, all controlled remotely, while the water pump activates when fire is detected.

  Author: Pipisara Chandrabhanu
*/


#include <SPI.h> // Include SPI library
#include <nRF24L01.h> // Include nRF24L01 library
#include <RF24.h> // Include RF24 library
#include <Servo.h> // Include Servo  Library

Servo TopServo;

Servo CamServo;

RF24 radio(2, 3);                
const byte address[6] = "00001";  // Address for the communication channel


#define LFM 9    // Left forward Motor
#define LBM 8    // Left backward Motor
#define RFM 11   // Right forward Motor
#define RBM 10   // Right backward Motor
#define LPWM 12  // Left PWM
#define RPWM 13  // Right PWM


#define LeftLED 22
#define RightLED 23
#define ReverseLED 24
#define FlameLed 25 // flame detect led
#define PumpLED 26  // pumpled
#define SLED 27   // Left Top_Servo
#define RSLED 28  // Right Top_Servo
#define WPLED 29  // Water pump
#define SignalLED 30

#define Flasher 40



#define Top_Servo 5
#define Cam_Servo 6


#define WPump 7        // Water pump pin
#define FlameSensor 42  // IR sensor pin
#define FlameSensor2 43 //second ir

int angle = 90;     // Top_Servo angle
int Camangle = 90;

int pumpstate = 0 ;  // Water pump state

int flasState = 0;  //flasher for cam

unsigned long currentTime;  // to blink light
unsigned long previousTime = 0;
const long interval = 200;
int ledStateblink = LOW;
int reset = 0;


void setup() {


  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();


  TopServo.attach(Top_Servo);
  TopServo.write(90);

  CamServo.attach(Cam_Servo);
  CamServo.write(90);

  for (int x = 22; x < 41; x++) {  // LED OUTPUT

    pinMode(x, OUTPUT);
  }



  pinMode(LFM, OUTPUT);
  pinMode(LBM, OUTPUT);
  pinMode(RFM, OUTPUT);
  pinMode(RBM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(WPump, OUTPUT);
  pinMode(FlameSensor, INPUT);
  pinMode(FlameSensor2, INPUT);

  TopServo.write(angle);
  analogWrite(LPWM, 155);
  analogWrite(RPWM, 155);
  digitalWrite(WPump, HIGH);
  digitalWrite(Flasher, HIGH);

  Serial.begin(9600); // Initialize serial communication
  SPI.begin(); // Initialize SPI communication
 
}



void loop() {




 if (reset == 0) {
    if (digitalRead(FlameSensor) == 0) {
      digitalWrite(FlameLed, HIGH);
    

      if (pumpstate == 0) {
        digitalWrite(WPump, LOW);
        digitalWrite(PumpLED, HIGH);
        Camangle=135;
        TopServo.write(Camangle);
        reset = 1;
      }
    }
    
    if (digitalRead(FlameSensor2) == 0) {
      digitalWrite(FlameLed, HIGH);
    
 
      if (pumpstate == 0) {
        digitalWrite(WPump, LOW);
        digitalWrite(PumpLED, HIGH);
        Camangle=45;
        TopServo.write(Camangle);
        reset = 1;
      }
    }
    
  }


   if (radio.available()) {
    char remoteData;
    radio.read(&remoteData, sizeof(remoteData));


    if (remoteData == 'F') {

      forward(); // Go Forward
      digitalWrite(LeftLED, HIGH);
      digitalWrite(RightLED, HIGH);
      digitalWrite(ReverseLED, LOW);
      digitalWrite(SignalLED, HIGH);
    }

    else if (remoteData == 'B') {

      backward(); //Go Backword
      digitalWrite(LeftLED, LOW);
      digitalWrite(RightLED, LOW);
      digitalWrite(ReverseLED, HIGH);
      digitalWrite(SignalLED, HIGH);
    }

    else if (remoteData == 'L') {

      turnLeft();  //Turn Left
      digitalWrite(LeftLED, HIGH);
      digitalWrite(RightLED, LOW);
      digitalWrite(ReverseLED, LOW);
      digitalWrite(SignalLED, HIGH);
    }

    else if (remoteData == 'R') {

      turnRight();  //Turn Right
      digitalWrite(LeftLED, LOW);
      digitalWrite(RightLED, HIGH);
      digitalWrite(ReverseLED, LOW);
      digitalWrite(SignalLED, HIGH);


    } else if (remoteData == 'A') {
      reset = 0;
      digitalWrite(SignalLED, HIGH);
      digitalWrite(SLED, HIGH);
      if (angle >= 0) {  
        angle--;
        delay(10);
        TopServo.write(angle);
      }

    }

    else if (remoteData == 'D') {

      reset = 0;
      digitalWrite(RSLED, HIGH);

      digitalWrite(SignalLED, HIGH);

      if (angle <= 180) {  // Ensure angle doesn't exceed Top_Servo's maximum angle
        angle++;
        delay(10);
        TopServo.write(angle);
      }


    }

    else if (remoteData == 'M') {

      digitalWrite(SignalLED, HIGH);

      if (Camangle >= 0) {  
        Camangle--;
        delay(10);
        CamServo.write(Camangle);
      }

    }

    else if (remoteData == 'N') {


      reset = 0;

      digitalWrite(SignalLED, HIGH);

      if (Camangle <= 180) {  // Ensure angle doesn't exceed Top_Servo's maximum angle
        Camangle++;
        delay(10);
        CamServo.write(Camangle);
      }


    }

    else if (remoteData == 'Y') { //to reset  state of fire detecting

      reset = 0;

      digitalWrite(SignalLED, HIGH);
      digitalWrite(FlameLed, LOW);


    }


     else if (remoteData == 'K') { //to reset  state of fire detecting

      

      digitalWrite(SignalLED, HIGH);
      Camangle = 90;
        delay(10);
        CamServo.write(Camangle);

    }



    else if (remoteData == 'S') {

      stop();
      digitalWrite(LeftLED, HIGH);
      digitalWrite(RightLED, HIGH);
      digitalWrite(SignalLED, LOW);
      currentTime = millis();     //Light Blinking 
      if (currentTime - previousTime >= interval) {
        previousTime = currentTime;
        if (ledStateblink == LOW) {
          ledStateblink = HIGH;
        } else {
          ledStateblink = LOW;
        }
        digitalWrite(ReverseLED, ledStateblink);
        digitalWrite(SignalLED, ledStateblink);
      }

      

    } else if (remoteData == 'W') {

     
      pumpstate = !pumpstate;

      digitalWrite(SignalLED, HIGH);
      delay(50);

      digitalWrite(SLED, HIGH);
      digitalWrite(PumpLED, pumpstate);
      digitalWrite(WPump, !pumpstate);

      return;
      
    }
    else if (remoteData == 'Z') {

      flasState = !flasState;

      digitalWrite(SignalLED, HIGH);
      delay(50);

      
      digitalWrite(Flasher, !flasState);

      return;
    }
  }
}






// Motor controller functions
void forward() {
  digitalWrite(LFM, HIGH);
  digitalWrite(RFM, HIGH);
  digitalWrite(LBM, LOW);
  digitalWrite(RBM, LOW);
}

void backward() {
  digitalWrite(LFM, LOW);
  digitalWrite(RFM, LOW);
  digitalWrite(LBM, HIGH);
  digitalWrite(RBM, HIGH);
}

void turnLeft() {
  digitalWrite(LFM, LOW);
  digitalWrite(RFM, HIGH);
  digitalWrite(LBM, HIGH);
  digitalWrite(RBM, LOW);
}

void turnRight() {
  digitalWrite(LFM, HIGH);
  digitalWrite(RFM, LOW);
  digitalWrite(LBM, LOW);
  digitalWrite(RBM, HIGH);
}

void stop() {
  digitalWrite(LFM, LOW);
  digitalWrite(RFM, LOW);
  digitalWrite(LBM, LOW);
  digitalWrite(RBM, LOW);
}
