#include <MsTimer2.h>

const byte ledPin = 4;
const byte buttonPin = 6;
const byte interruptPin = 2;
const unsigned long ledOnDurationMs = 5000;

void turnOffLed() {
  // This callback runs when the MsTimer2 timer expires.
  digitalWrite(ledPin, LOW);
}

void handleButtonPress() {
  // Turn the LED on immediately when the interrupt fires, then start the timer.
  digitalWrite(ledPin, HIGH);
  MsTimer2::start();
}

void setup() {
  // Configure the LED and button pins.
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(interruptPin, INPUT);
  digitalWrite(ledPin, LOW);

  // Prepare the timer to call turnOffLed() after 5 seconds.
  MsTimer2::set(ledOnDurationMs, turnOffLed);
  MsTimer2::stop();

  // Trigger the ISR from the interrupt pin that is shorted to the button pin.
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonPress, RISING);
  Serial.begin(9600);
}

void loop() {
  // Simulate a long process so we can see that the timer still works.
  for (int i = 0; i < 10000; i++) {
    Serial.println("calculating...");
  }

  delay(1000);
}
