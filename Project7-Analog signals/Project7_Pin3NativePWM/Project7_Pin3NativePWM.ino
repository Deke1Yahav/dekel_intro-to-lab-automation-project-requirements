const byte rotaryPin = A0;
const byte ledPin = 3;
const byte shortedPin = 4;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  // pin4 is wired directly to pin3 (which drives the LED via native PWM).
  // Keep it as an input so it doesn't fight pin3's output.
  pinMode(shortedPin, INPUT);
}

void loop() {
  int rotaryValue = analogRead(rotaryPin);
  byte dutyCycle = map(rotaryValue, 0, 1023, 0, 255);
  analogWrite(ledPin, dutyCycle);

  Serial.println(rotaryValue);
  delay(100);
}
