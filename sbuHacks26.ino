#include "thingProperties.h"
#include <Servo.h>

Servo sg90;

const int SERVO_PIN = 8;

const int REST_ANGLE = 20;
const int EXTEND_ANGLE = 90;

const unsigned long SERVO_STEP_MS = 600;

// Trigger at or above 1.0 m/s
const float VELOCITY_THRESHOLD = 1.0f;

// Must drop below this before re-arming
const float REARM_THRESHOLD = 0.8f;


bool servoRunning = false;
bool armed = false;

int servoStep = 0;
unsigned long lastServoStep = 0;

void setup() {

  Serial.begin(9600);
  delay(1500);

  // Servo initialization
  sg90.attach(SERVO_PIN);
  sg90.write(REST_ANGLE);

  // Arduino Cloud initialization
  initProperties();

  ArduinoCloud.begin(
    ArduinoIoTPreferredConnection
  );

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("System initialized");
}



void loop() {

  ArduinoCloud.update();

  // testSpeedMps is entered on the dashboard.
  // It is NOT calculated from acceleration.
  float speed = testSpeedMps;

  // Only re-arm below 0.8 m/s
  if (!servoRunning &&
      speed < REARM_THRESHOLD) {
    armed = true;
  }

  // Trigger on threshold crossing
  if (armed &&
      !servoRunning &&
      speed >= VELOCITY_THRESHOLD) {

    armed = false;
    startServoSequence();
  }

  // Continue servo sequence without delay()
  updateServo();

  // Print measurements every 500 ms
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 500) {
    lastPrint = millis();

    Serial.print("iPhone accel: ");
    Serial.print(linear_accel, 3);

    Serial.print(" | Test speed: ");
    Serial.print(speed, 3);

    Serial.println(" m/s");
  }
}



void startServoSequence() {

  servoRunning = true;
  servoStep = 1;

  lastServoStep = millis();

  // Begin first outward movement
  sg90.write(EXTEND_ANGLE);

  Serial.println("VELOCITY THRESHOLD REACHED");
  Serial.println("Starting servo sequence");
}


void updateServo() {

  if (!servoRunning) {
    return;
  }

  if (millis() - lastServoStep < SERVO_STEP_MS) {
    return;
  }

  lastServoStep = millis();

  switch (servoStep) {

    case 1:
      // First return
      sg90.write(REST_ANGLE);
      servoStep = 2;
      break;

    case 2:
      // Second outward movement
      sg90.write(EXTEND_ANGLE);
      servoStep = 3;
      break;

    case 3:
      // Second return
      sg90.write(REST_ANGLE);
      servoStep = 4;
      break;

    case 4:
      servoRunning = false;
      servoStep = 0;

      Serial.println("Servo sequence complete");
      break;
  }
}


// Invoked when testSpeedMps changes through Cloud
void onTestSpeedMpsChange() {
  Serial.print("New test speed: ");
  Serial.println(testSpeedMps);
}

// Used if the synchronized variable has
// an on-change callback in thingProperties.h
void onLinearAccelChange() {
  Serial.print("New iPhone acceleration: ");
  Serial.println(linear_accel, 4);
}
 