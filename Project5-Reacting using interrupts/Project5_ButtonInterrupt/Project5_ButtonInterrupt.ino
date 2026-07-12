const byte buttonPin = 6;
const byte ledPin = 4;
const byte interruptPin = 2;

volatile bool ledState = false;

void handleButtonInterrupt() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState ? HIGH : LOW);
}

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(interruptPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleButtonInterrupt, CHANGE);
}

void loop() {
  Serial.println("Waiting for button interrupt...");
  for (int i = 0; i < 10000; i++) {
    Serial.println("calculating...");
  }
  delay(100);
}
