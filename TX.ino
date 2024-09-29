#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7,6); // CE, CSN
const byte address[6] = "00011";

#define pushButtonPin 5

/* this "package" is like an object(storage)
for the data that we shall write in the communication pipe*/

struct package  
{
  int xAxis;
  int yAxis;
  //int lastButtonState;
  int currentButtonState;
};

package one;   // "one" is the name of the package

void resetData()
{
  one.xAxis = 500;
  one.yAxis = 500;
  //one.currentButtonState = digitalRead(pushButtonPin);
  //one.lastButtonState = one.currentButtonState;
}

void setup() {

  pinMode(pushButtonPin, INPUT_PULLUP);
  
  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  resetData(); 
  /*in the event that there is no input from the user, 
   then the values in this void are taken as the current values*/
}

void loop() {

  one.currentButtonState = digitalRead(pushButtonPin); // read new state
 // one.lastButtonState    = one.currentButtonState;      // save the last state
  
  
  one.xAxis = analogRead(A1); // Read Joysticks X-axis
  one.yAxis = analogRead(A0); // Read Joysticks Y-axis

  radio.write(&one, sizeof(package));

  delay(10);
}
