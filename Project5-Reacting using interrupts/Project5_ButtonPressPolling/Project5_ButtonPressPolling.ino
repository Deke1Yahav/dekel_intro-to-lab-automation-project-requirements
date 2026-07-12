const byte buttonPin = 6;
const byte ledPin = 4;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Button pressed");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("Button released");
  }

  for (int i = 0; i < 10000; i++) {
    Serial.println("calculating...");
  }

  delay(100);
}
