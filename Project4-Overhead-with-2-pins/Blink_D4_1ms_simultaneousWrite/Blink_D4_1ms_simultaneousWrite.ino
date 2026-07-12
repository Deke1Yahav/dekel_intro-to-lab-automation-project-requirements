const byte firstLedPin = 13;
const byte secondLedPin = 12;
const byte ledPinsMask = _BV(PB4) | _BV(PB5);
const unsigned int blinkDelayMs = 1;

void setup() {
  // Pins 13 and 12 are on PORTB, so we can change both bits with one write.
  DDRB |= ledPinsMask;
}

void loop() {
  // Turn both pins on together, wait, then turn both pins off together.
  PORTB |= ledPinsMask;
  delay(blinkDelayMs);
  PORTB &= ~ledPinsMask;
  delay(blinkDelayMs);
}