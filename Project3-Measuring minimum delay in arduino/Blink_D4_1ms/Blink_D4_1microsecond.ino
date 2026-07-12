void setup() {
  pinMode(4, OUTPUT);
}

void loop() {
  digitalWrite(4, HIGH);
  delayMicroseconds(1);
  digitalWrite(4, LOW);
  delayMicroseconds(1);
}