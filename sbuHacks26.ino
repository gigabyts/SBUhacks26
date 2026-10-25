
#include "thingProperties.h"
#include <Servo.h>
#include <math.h>

// =====================================
// CONFIGURATION
// =====================================

const int SERVO_PIN = 9;

// Commanded servo travel: 0 to 180 degrees
const int REST_ANGLE = 0;
const int ACTIVE_ANGLE = 180;

// Allow time for the servo to move
const unsigned long SERVO_INTERVAL = 750;

// Two velocity-proxy thresholds
const float LOW_VELOCITY_THRESHOLD = 0.75f;
const float HIGH_VELOCITY_THRESHOLD = 2.25f;

// Acceleration-based fallback thresholds
const bool ENABLE_SHAKE_FALLBACK = true;

const float LOW_SHAKE_THRESHOLD_G = 1.5f;
const float HIGH_SHAKE_THRESHOLD_G = 3.0f;

// Acceleration settings
const float GRAVITY = 9.80665f;
const float RESTING_G = 1.0f;
const float NOISE_TOLERANCE = 0.05f;

// Integration settings
const unsigned long MAX_SAMPLE_GAP = 2000;
const float MAX_INTEGRATION_DT = 0.15f;

// Automatic rearming
const unsigned long REARM_STILL_TIME = 2000;
const float STILL_TOLERANCE_G = 0.08f;

// =====================================
// GLOBAL VARIABLES
// =====================================

Servo sg90;

// Numerical integration
float velocityProxy = 0.0f;
float previousAcceleration = 0.0f;

unsigned long previousSampleTime = 0;
bool havePreviousSample = false;

// Trigger logic
bool armed = true;
bool triggered = false;

// Servo sequence
bool servoRunning = false;
bool movingOut = false;

int targetCycles = 0;
int completedCycles = 0;

unsigned long previousServoTime = 0;

// Automatic reset
bool countingStillness = false;
unsigned long stillStartTime = 0;

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  sg90.attach(SERVO_PIN);
  sg90.write(REST_ANGLE);

  initProperties();

  ArduinoCloud.begin(
    ArduinoIoTPreferredConnection
  );

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("==========================");
  Serial.println("MOTIONGUARD READY");
  Serial.println("==========================");

  Serial.println("LOW: 1 cycle");
  Serial.println("HIGH: 2 cycles");
  Serial.println("Automatic rearming enabled.");
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  ArduinoCloud.update();

  // Nonblocking servo control
  updateServo();

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 500) {

    lastPrint = millis();

    Serial.print("Acceleration: ");
    Serial.print(linAccel, 3);
    Serial.print(" g");

    Serial.print(" | Velocity proxy: ");
    Serial.print(velocityProxy, 3);

    Serial.print(" | Armed: ");
    Serial.print(armed);

    Serial.print(" | Cycles: ");
    Serial.print(completedCycles);
    Serial.print("/");
    Serial.println(targetCycles);
  }
}

// =====================================
// ACCELERATION CONVERSION
// =====================================

float convertAcceleration(float gReading) {

  float excessG = gReading - RESTING_G;

  if (excessG < NOISE_TOLERANCE) {
    return 0.0f;
  }

  return excessG * GRAVITY;
}

// =====================================
// NUMERICAL INTEGRATION
// =====================================

void integrateAcceleration(
    float acceleration,
    unsigned long now) {

  if (!havePreviousSample) {

    previousAcceleration = acceleration;
    previousSampleTime = now;
    havePreviousSample = true;

    return;
  }

  unsigned long elapsed =
      now - previousSampleTime;

  previousSampleTime = now;

  if (elapsed == 0) {
    return;
  }

  if (elapsed > MAX_SAMPLE_GAP) {

    velocityProxy = 0.0f;
    previousAcceleration = acceleration;

    Serial.println("Integration reset: long gap");
    return;
  }

  float dt = elapsed / 1000.0f;

  // Avoid assigning a long Cloud interval
  // to a brief acceleration spike
  if (dt > MAX_INTEGRATION_DT) {
    dt = MAX_INTEGRATION_DT;
  }

  // Trapezoidal numerical integration
  float deltaV =
      0.5f *
      (previousAcceleration + acceleration) *
      dt;

  velocityProxy += deltaV;

  previousAcceleration = acceleration;

  Serial.print("Integrated proxy: ");
  Serial.println(velocityProxy, 4);
}

// =====================================
// CLOUD ACCELERATION CALLBACK
// =====================================

void onLinAccelChange() {

  unsigned long now = millis();
  float gReading = linAccel;

  if (!isfinite(gReading) ||
      gReading < 0.0f ||
      gReading > 20.0f) {
    return;
  }

  Serial.print("Received acceleration: ");
  Serial.print(gReading, 4);
  Serial.println(" g");

  // If a sequence has finished,
  // check whether the phone is at rest
  // and we can re-arm the system.
  if (triggered && !servoRunning) {

    checkAutomaticRearm(gReading, now);
    return;
  }

  if (!armed && !servoRunning) {
    return;
  }

  // ---------------------------------
  // Update numerical integration
  // ---------------------------------

  float acceleration =
      convertAcceleration(gReading);

  integrateAcceleration(acceleration, now);

  // ---------------------------------
  // Check HIGH threshold first
  // ---------------------------------

  bool highDetected =
      velocityProxy >= HIGH_VELOCITY_THRESHOLD;

  if (ENABLE_SHAKE_FALLBACK &&
      gReading >= HIGH_SHAKE_THRESHOLD_G) {
    highDetected = true;
  }

  // ---------------------------------
  // Check LOW threshold
  // ---------------------------------

  bool lowDetected =
      velocityProxy >= LOW_VELOCITY_THRESHOLD;

  if (ENABLE_SHAKE_FALLBACK &&
      gReading >= LOW_SHAKE_THRESHOLD_G) {
    lowDetected = true;
  }

  // ---------------------------------
  // Upgrade an active LOW sequence
  // if a HIGH reading arrives.
  // ---------------------------------

  if (servoRunning) {

    if (highDetected && targetCycles == 1) {

      targetCycles = 2;

      Serial.println("========================");
      Serial.println("UPGRADED TO HIGH!");
      Serial.println("Now performing 2 cycles.");
      Serial.println("========================");
    }

    return;
  }

  // ---------------------------------
  // Start a new sequence
  // ---------------------------------

  if (highDetected) {

    Serial.println("========================");
    Serial.println("HIGH THRESHOLD EXCEEDED!");
    Serial.println("========================");

    startServoSequence(2);

  } else if (lowDetected) {

    Serial.println("========================");
    Serial.println("LOW THRESHOLD EXCEEDED!");
    Serial.println("========================");

    startServoSequence(1);
  }
}

// =====================================
// START SERVO SEQUENCE
// =====================================

void startServoSequence(int cycles) {

  if (triggered || servoRunning) {
    return;
  }

  triggered = true;
  armed = false;

  servoRunning = true;
  movingOut = true;

  targetCycles = cycles;
  completedCycles = 0;

  previousServoTime = millis();

  countingStillness = false;
  stillStartTime = 0;

  // Start first forward rotation
  sg90.write(ACTIVE_ANGLE);

  Serial.print("Servo activated! Target cycles: ");
  Serial.println(targetCycles);
}

// =====================================
// SERVO MOVEMENT
// =====================================

void updateServo() {

  if (!servoRunning) {
    return;
  }

  unsigned long now = millis();

  if (now - previousServoTime < SERVO_INTERVAL) {
    return;
  }

  previousServoTime = now;

  if (movingOut) {

    // Return from 180 degrees to 0 degrees
    sg90.write(REST_ANGLE);

    movingOut = false;
    completedCycles++;

    Serial.print("Completed return: ");
    Serial.println(completedCycles);

  } else {

    // Start another cycle if required
    if (completedCycles < targetCycles) {

      sg90.write(ACTIVE_ANGLE);

      movingOut = true;

      Serial.println("Starting next cycle.");

    } else {

      // All requested cycles completed
      servoRunning = false;

      Serial.println("========================");
      Serial.println("SERVO SEQUENCE COMPLETE");
      Serial.println("========================");

      Serial.println("Waiting for stillness...");
    }
  }
}

// =====================================
// AUTOMATIC REARMING
// =====================================

void checkAutomaticRearm(
    float gReading,
    unsigned long now) {

  bool isStill =
      fabsf(gReading - RESTING_G)
      <= STILL_TOLERANCE_G;

  if (isStill) {

    if (!countingStillness) {

      countingStillness = true;
      stillStartTime = now;
    }

    if (now - stillStartTime >=
        REARM_STILL_TIME) {

      resetDetection();
    }

  } else {

    countingStillness = false;
    stillStartTime = 0;
  }
}

// =====================================
// RESET DETECTION
// =====================================

void resetDetection() {

  velocityProxy = 0.0f;
  previousAcceleration = 0.0f;

  havePreviousSample = false;
  previousSampleTime = 0;

  armed = true;
  triggered = false;

  targetCycles = 0;
  completedCycles = 0;

  countingStillness = false;
  stillStartTime = 0;

  Serial.println("========================");
  Serial.println("SYSTEM RE-ARMED");
  Serial.println("Ready for next movement.");
  Serial.println("========================");
}
