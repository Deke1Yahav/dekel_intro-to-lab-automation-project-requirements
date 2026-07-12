const byte firstLedPin = 13;
const byte secondLedPin = 12;
const unsigned int blinkDelayMs = 1;

int calculationValue = 0;

void setup() {
  pinMode(firstLedPin, OUTPUT);
  pinMode(secondLedPin, OUTPUT);
}

void loop() {
  digitalWrite(firstLedPin, HIGH);
  calculationValue = calculationValue + 1;
  digitalWrite(secondLedPin, HIGH);
  delay(blinkDelayMs);

  digitalWrite(firstLedPin, LOW);
  calculationValue = calculationValue + 1;
  digitalWrite(secondLedPin, LOW);
  delay(blinkDelayMs);
}