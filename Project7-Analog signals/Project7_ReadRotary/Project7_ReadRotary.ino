#include <MD_PWM.h>

const byte rotaryPin = A0;
const byte ledPin = 4;
const uint16_t pwmFrequencyHz = 50;

// MD_PWM writes duty cycle as a byte: 0 = 0%, 255 = 100%.
MD_PWM ledPwm(ledPin);

void setup() {
  Serial.begin(9600);
  ledPwm.begin(pwmFrequencyHz);
}

void loop() {
  int rotaryValue = analogRead(rotaryPin);
  byte dutyCycle = map(rotaryValue, 0, 1023, 0, 255);
  ledPwm.write(dutyCycle);

  Serial.println(rotaryValue);
  delay(100);
}
