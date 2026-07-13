const byte ledPin = 4;
const byte interruptPin = 2;
const unsigned long ledOnDurationMs = 5000;

volatile bool ledRequest = false;
unsigned long ledTurnedOnAt = 0;
bool ledIsOn = false;

void handleButtonInterrupt() {
  ledRequest = true;
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonInterrupt, RISING);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
}

void loop() {
  if (ledRequest) {
    noInterrupts();
    ledRequest = false;
    interrupts();

    digitalWrite(ledPin, HIGH);
    ledTurnedOnAt = millis();
    ledIsOn = true;
  }

  if (ledIsOn && (millis() - ledTurnedOnAt >= ledOnDurationMs)) {
    digitalWrite(ledPin, LOW);
    ledIsOn = false;
  }

  for (int i = 0; i < 10000; i++) {
    Serial.println("calculating...");
  }
}
