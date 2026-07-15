// Tilt-aware fan system (Project 11): an accelerometer-driven servo tilts the fan,
// a buzzer/motor cut-out guards against excess tilt, and the state is sent over
// serial each tick for the companion Python GUI/logger to display and log.
#include <Arduino_SensorKit.h>
#include <Servo.h>

// ---- Pin configuration ----
const byte SERVO_PIN = 3;       // external servo, tilts the fan
const byte FAN_MOTOR_PIN = 7;   // external DC fan motor, on/off only
const byte BUZZER_PIN = 5;      // onboard buzzer (fixed by the Grove Beginner Kit)
// Accelerometer and OLED are onboard, wired to I2C, and exposed by Arduino_SensorKit
// as the ready-made `Accelerometer` and `Oled` objects (no pins to declare).

// ---- Tuning constants ----
const float TILT_THRESHOLD_DEG = 30.0;   // beyond this, the alarm state kicks in
const float TILT_DEADBAND_DEG = 2.5;     // readings within +-this are treated as level
const int SERVO_MIN_DEG = 10;            // safe sweep, matches Project 7's tested range
const int SERVO_MAX_DEG = 170;
const unsigned int BUZZER_FREQUENCY_HZ = 2000;
const unsigned long LOOP_INTERVAL_MS = 50;
const unsigned long SERIAL_BAUD = 9600;

Servo fanServo;

// Reads the accelerometer and returns tilt in degrees around one axis.
// Assumes the board sits with Z pointing up when level; swap axes here if mounted differently.
float readTiltDegrees() {
  float x = Accelerometer.readX();
  float z = Accelerometer.readZ();
  float tiltDeg = atan2(x, z) * 180.0 / PI;

  if (fabs(tiltDeg) <= TILT_DEADBAND_DEG) {
    tiltDeg = 0.0; // ignore noise/jitter around level
  }
  return tiltDeg;
}

// Maps a tilt reading (clamped to +-90 deg) onto the servo's safe sweep.
int tiltToServoAngle(float tiltDeg) {
  float clamped = constrain(tiltDeg, -90.0, 90.0);
  return map((long)clamped, -90, 90, SERVO_MIN_DEG, SERVO_MAX_DEG);
}

// Drives the servo, fan motor, and buzzer according to the current state.
// In the alarm state the servo is simply not updated, so it holds the last safe angle.
void updateActuators(float tiltDeg, bool alarmActive) {
  if (alarmActive) {
    digitalWrite(FAN_MOTOR_PIN, LOW);
    tone(BUZZER_PIN, BUZZER_FREQUENCY_HZ);
  } else {
    fanServo.write(tiltToServoAngle(tiltDeg));
    digitalWrite(FAN_MOTOR_PIN, HIGH);
    noTone(BUZZER_PIN);
  }
}

// Shows the live angle and state on the OLED, padded to overwrite shorter previous text.
void updateDisplay(float tiltDeg, bool alarmActive) {
  Oled.setFont(u8x8_font_chroma48medium8_r);

  Oled.setCursor(0, 0);
  Oled.print("Angle: ");
  Oled.print(tiltDeg, 1);
  Oled.print(" deg   ");

  Oled.setCursor(0, 2);
  Oled.print(alarmActive ? "ALARM   " : "OK      ");
}

// Sends one CSV-style line per tick: angle in degrees, buzzer state as 0/1.
void sendTelemetry(float tiltDeg, bool alarmActive) {
  Serial.print(tiltDeg, 1);
  Serial.print(",");
  Serial.println(alarmActive ? 1 : 0);
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  Accelerometer.begin();
  Oled.begin();
  Oled.setFlipMode(true);

  fanServo.attach(SERVO_PIN);
  pinMode(FAN_MOTOR_PIN, OUTPUT);
  digitalWrite(FAN_MOTOR_PIN, HIGH); // start in the running state
}

void loop() {
  float tiltDeg = readTiltDegrees();
  bool alarmActive = fabs(tiltDeg) > TILT_THRESHOLD_DEG;

  updateActuators(tiltDeg, alarmActive);
  updateDisplay(tiltDeg, alarmActive);
  sendTelemetry(tiltDeg, alarmActive);

  delay(LOOP_INTERVAL_MS);
}
