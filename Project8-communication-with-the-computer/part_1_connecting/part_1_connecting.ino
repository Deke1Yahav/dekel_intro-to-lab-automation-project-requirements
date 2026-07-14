#include <MsTimer2.h>

// Pin definitions
const byte LED_PIN = 4;      // LED connected to digital pin 4
const byte BUTTON_PIN = 2;   // Button bridged to pin 2 (supports hardware interrupt)

// Global variables
volatile unsigned long ledOnDuration = 1000; // Default duration: 1000ms
volatile bool isLedActive = false;           // Flag to track if LED is currently ON

// Debouncing variables
volatile unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;    // Ignored button presses within 200ms

// Function declarations
void buttonInterrupt();
void turn_off();

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Make sure LED starts OFF
  
  // Initialize button pin with internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Attach external interrupt to pin 2 on falling edge (button pressed)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
}

void loop() {
  // Continuously check for incoming serial data
  if (Serial.available() > 0) {
    // Read string until newline character '\n'
    String incomingString = Serial.readStringUntil('\n');
    incomingString.trim(); // Remove any leading/trailing whitespace
    
    // Parse the string into a long integer
    long parsedTime = incomingString.toInt();
    
    // Validate input (must be a positive number)
    if (parsedTime > 0) {
      // Safely update the shared volatile variable
      noInterrupts(); // Temporarily disable interrupts to prevent race conditions
      ledOnDuration = parsedTime;
      interrupts();   // Re-enable interrupts
      
      // Send confirmation back to PC
      Serial.print("I received: ");
      Serial.println(ledOnDuration);
    } else {
      Serial.println("Error: Invalid number received.");
    }
  }
}

// Interrupt Service Routine (ISR) triggered on button press
void buttonInterrupt() {
  unsigned long currentTime = millis();
  
  // Software debouncing logic
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    if (!isLedActive) { // Only activate if LED is not already ON
      isLedActive = true;
      digitalWrite(LED_PIN, HIGH);
      
      // Notify Python GUI: '1' indicates Button pressed and LED ON
      Serial.println("1");
      
      // Set the MsTimer2 timer with the received duration + 1ms bugfix
      MsTimer2::set(ledOnDuration + 1, turn_off);
      MsTimer2::start();
    }
    lastDebounceTime = currentTime;
  }
}

// Callback function executed when MsTimer2 expires
void turn_off() {
  digitalWrite(LED_PIN, LOW);
  MsTimer2::stop(); // Stop the timer
  isLedActive = false;
  
  // Notify Python GUI: '0' indicates LED is now OFF
  Serial.println("0");
}