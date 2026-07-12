/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/
const byte firstLedPin = 13;
const byte secondLedPin = 12;
const unsigned int blinkDelayMs = 1;

void setup() {
  pinMode(firstLedPin, OUTPUT);
  pinMode(secondLedPin, OUTPUT);
}

void loop() {
  digitalWrite(firstLedPin, HIGH);
  digitalWrite(secondLedPin, HIGH);
  delay(blinkDelayMs);
  digitalWrite(firstLedPin, LOW);
  digitalWrite(secondLedPin, LOW);
  delay(blinkDelayMs);
}
