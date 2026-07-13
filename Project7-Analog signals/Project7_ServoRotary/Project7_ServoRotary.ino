#include <Servo.h>

const byte rotaryPin = A0;
const byte servoPin = 7;

Servo myServo;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
}

void loop() {
  int rotaryValue = analogRead(rotaryPin);
  int angle = map(rotaryValue, 0, 1023, 0, 180);
  myServo.write(angle);

  Serial.println(angle);
  delay(100);
}
