#include <Servo.h>
#include <Wire.h>
#include <math.h>
#include "MMA7660.h"

MMA7660 accelemeter;
Servo winchServo;
int pos = 0;

findVelocity
void setup() {
  // put your setup code here, to run once:
  winchServo.attach(8);
  accelemeter.init();  
	Serial.begin(9600);


}

void loop() {
  // put your main code here, to run repeatedly:
  int8_t x;
	int8_t y;
	int8_t z;
	float ax,ay,az;
	accelemeter.getXYZ(&x,&y,&z);
	
	Serial.print("x = ");
  Serial.println(x); 
  Serial.print("y = ");
  Serial.println(y);   
  Serial.print("z = ");
  Serial.println(z);
	
	accelemeter.getAcceleration(&ax,&ay,&az);
  Serial.println("accleration of X/Y/Z: ");
	Serial.print(ax);
	Serial.println(" g");
	Serial.print(ay);
	Serial.println(" g");
	Serial.print(az);
	Serial.println(" g");
	Serial.println("*************");
	delay(500);
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    winchServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    winchServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }


}
