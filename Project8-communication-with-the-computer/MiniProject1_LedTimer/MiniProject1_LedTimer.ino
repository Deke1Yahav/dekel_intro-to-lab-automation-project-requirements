#include <MsTimer2.h>

// ---- Pin configuration ----
const byte LED_PIN = 4;     // LED lit for a computer-configured duration
const byte BUTTON_PIN = 2;  // Must be an interrupt-capable pin (D2 on Uno/Nano)

// ---- Serial protocol ----
// Single-character status codes sent to the computer after every button event.
const char STATE_LED_OFF = '0';         // Timer expired, LED turned off
const char STATE_BUTTON_PRESSED = '1';  // Button pressed, LED turned on
const char STATE_BUTTON_RELEASED = '2'; // Button released

// ---- Shared state (written in the ISR, read in loop, so it's volatile) ----
volatile unsigned long ledOnDurationMs = 1000; // Updated by the computer over serial
volatile bool ledIsOn = false;

// ---- Debouncing ----
volatile unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY_MS = 50;

void turnOffLed() {
  // MsTimer2 callback: fires once ledOnDurationMs has elapsed since the button press.
  digitalWrite(LED_PIN, LOW);
  MsTimer2::stop();
  ledIsOn = false;
  Serial.println(STATE_LED_OFF);
}

void handleButtonChange() {
  // Runs on every edge of BUTTON_PIN (press and release), debounced in software.
  unsigned long now = millis();
  if (now - lastDebounceTime < DEBOUNCE_DELAY_MS) {
    return; // Ignore bounce within the debounce window
  }
  lastDebounceTime = now;

  bool isPressed = (digitalRead(BUTTON_PIN) == LOW); // INPUT_PULLUP: LOW means pressed

  if (isPressed && !ledIsOn) {
    ledIsOn = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println(STATE_BUTTON_PRESSED);

    // +1ms works around the MsTimer2 off-by-one seen in Project6.
    MsTimer2::set(ledOnDurationMs + 1, turnOffLed);
    MsTimer2::start();
  } else if (!isPressed) {
    Serial.println(STATE_BUTTON_RELEASED);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonChange, CHANGE);
}

void loop() {
  // Non-blocking read of a newline-terminated number from the computer.
  if (Serial.available() > 0) {
    String incoming = Serial.readStringUntil('\n');
    incoming.trim();

    long parsedMs = incoming.toInt();

    // toInt() returns 0 for both "0" and unparseable text, so require a positive value.
    if (parsedMs > 0) {
      noInterrupts(); // Guard the write since handleButtonChange() also reads this
      ledOnDurationMs = (unsigned long)parsedMs;
      interrupts();

      Serial.print("I received: ");
      Serial.println(ledOnDurationMs);
    } else {
      Serial.print("Error: invalid duration '");
      Serial.print(incoming);
      Serial.println("', expected a positive integer in ms.");
    }
  }
}
