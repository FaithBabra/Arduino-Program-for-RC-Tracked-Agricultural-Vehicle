#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define enA 10
#define enB 11
#define in1 2
#define in2 3
#define in3 4
#define in4 5

RF24 radio (7,9); //CE,CSN
const byte address[6] = "00011";

int motorSpeedA = 0;
int motorSpeedB = 0;

//for red LED
const int led_pin_red = 8;
const int led_pin_night = 6; // Arduino pin connected to LED's pin

int ledState = LOW;     // the current state of LED
int lastButtonState = 1;

struct package
{
  int xAxis;
  int yAxis;
  //int lastButtonState;      // the previous state of button
  int currentButtonState;   // the current state of button
};

package one;

void resetData()
{
  one.xAxis = 500;
  one.yAxis = 500;
  //one.lastButtonState = one.currentButtonState;
  digitalWrite(led_pin_red, LOW);
  //digitalWrite(led_pin_night, ledState); 
}

void setup() {
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(led_pin_red, OUTPUT);
  pinMode(led_pin_night, OUTPUT);

  Serial.begin(115200);
  resetData();
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

unsigned long lastRecvTime = 0; //necessary in preparing for a max wait time

void recvData()
{  
  while ( radio.available() ) 
  {        
    radio.read(&one, sizeof(package));
    lastRecvTime = millis();
  }
}


void loop() {

  recvData();
  
  unsigned long now = millis();
  if ( (now - lastRecvTime) > 1000 ) {
    /*if this condition is true, then signal has been lost, 
    so data should all be reset */
    resetData();
  }

   if(lastButtonState == LOW && one.currentButtonState == HIGH) {
    Serial.println("The button is pressed");

    // toggle state of LED
    ledState = !ledState;

    // control LED arccoding to the toggled state
    digitalWrite(led_pin_night, ledState); 
  }

  lastButtonState = one.currentButtonState;
  

  //Y-axis is for forward and back movement
  if (one.yAxis<470) {

    //for LED 
     digitalWrite(led_pin_red, HIGH);
     
    //Set Motor A backward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    //Set Motor B backward
    digitalWrite(in3, LOW);
    digitalWrite(in4,HIGH);
    /*Convert the declining Y-axis readings from (470-0) into (0-100)
    value for the PWM signal for increasing the motor speed*/
    motorSpeedA = map(one.yAxis, 470, 0, 0, 100);
    motorSpeedB = map(one.yAxis, 470, 0, 0, 100);
  }
  
   else if (one.yAxis > 550) {

     //for LED
    digitalWrite(led_pin_red, LOW);
    
    // Set Motor A forward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    // Set Motor B forward
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    /*Convert the increasing Y-axis readings for going forward from 550 to 
    1023 into 0 to 100 value for the PWM signal for increasing the motor speed*/
    motorSpeedA = map(one.yAxis, 550, 1023, 0, 100);
    motorSpeedB = map(one.yAxis, 550, 1023, 0, 100);
   }

  //if joystick stays in the middle, the motors should have 0 speed
  else{
    motorSpeedA = 0;
    motorSpeedB = 0;
  }
  
  //X- axis for left and right control
  if(one.xAxis < 470){

     //for LED
    digitalWrite(led_pin_red, LOW);
    
    /*Convert the declining X-axis readings from (470-0) to (0-100)value for 
    the PWM signal for increasing the motor speed*/
    
    int xMapped = map(one.xAxis, 470, 0, 0, 100);

    //move left: decrease left motor speed and increase right motor speed
    motorSpeedA += xMapped;
    motorSpeedB -= xMapped;
    //confine the range from 0 to 100
    if (motorSpeedA > 100){
      motorSpeedA = 100;
    }
    if (motorSpeedB <0) {
      motorSpeedB = 0;
    }
  }

  if (one.xAxis >550) {

     //for LED
    digitalWrite(led_pin_red, LOW);
    
    //convert the increasing X-axis readings from (550-1023) into (0-100) for speed
    int xMapped = map(one.xAxis, 550, 1023, 0, 100);

    //move right: decrease right motor speed and increase left motor speed
    motorSpeedA -= xMapped;
    motorSpeedB += xMapped;
    //confine the speed range to 0-100
    if (motorSpeedA < 0) {
      motorSpeedA = 0;
    }
    if(motorSpeedB > 100) {
      motorSpeedB = 100;
    }
  }

  //preventing buzzing at low speeds
  if (motorSpeedA < 20) {
    motorSpeedA = 0;
  }
  if (motorSpeedB < 20) {
    motorSpeedB = 0;
  }
  
  analogWrite(enA, motorSpeedA); //Send PWM signal to motor A
  analogWrite(enB, motorSpeedB); //Send PWM signal to motor B
}
