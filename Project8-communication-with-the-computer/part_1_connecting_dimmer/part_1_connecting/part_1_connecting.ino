#include <MsTimer2.h>

// Pin definitions
const byte PWM_LED_PIN = 5;  // Moved to D5 (PWM) to avoid conflict with MsTimer2 on Timer2
const byte MONITOR_PIN = 4;  // Pin D4 configured as INPUT to monitor D5
const byte BUTTON_PIN = 2;   // Button bridged to pin D2 (hardware interrupt)

// Global variables
volatile unsigned long ledOnDuration = 1000; // Default duration: 1000ms
volatile byte ledBrightness = 255;           // Default brightness: 100% (255)
volatile bool isLedActive = false;           // Track if LED is active

// Debouncing variables
volatile unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// Function declarations
void buttonInterrupt();
void turn_off();

void setup() {
  Serial.begin(9600);
  
  // Configure LED pin as output and start as OFF
  pinMode(PWM_LED_PIN, OUTPUT);
  analogWrite(PWM_LED_PIN, 0);
  
  // Configure D4 as INPUT as requested
  pinMode(MONITOR_PIN, INPUT);
  
  // Button setup with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
}

void loop() {
  if (Serial.available() > 0) {
    String incomingString = Serial.readStringUntil('\n');
    incomingString.trim();
    
    int commaIndex = incomingString.indexOf(',');
    
    if (commaIndex != -1) {
      String durationPart = incomingString.substring(0, commaIndex);
      String brightnessPart = incomingString.substring(commaIndex + 1);
      
      long parsedTime = durationPart.toInt();
      int parsedBrightness = brightnessPart.toInt();
      
      if (parsedTime > 0 && parsedBrightness >= 0 && parsedBrightness <= 255) {
        noInterrupts();
        ledOnDuration = parsedTime;
        ledBrightness = (byte)parsedBrightness;
        interrupts();
        
        // IMMEDIATE EFFECT: If the LED is currently active, update its brightness instantly!
        if (isLedActive) {
          analogWrite(PWM_LED_PIN, ledBrightness);
        }
        
        Serial.print("I received: ");
        Serial.print(ledOnDuration);
        Serial.print(" ms with brightness: ");
        Serial.println(ledBrightness);
      } else {
        Serial.println("Error: Invalid parameters.");
      }
    }
  }
}

void buttonInterrupt() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    if (!isLedActive) {
      isLedActive = true;
      
      // Write the brightness using PWM on D5
      analogWrite(PWM_LED_PIN, ledBrightness);
      
      // Notify Python GUI
      Serial.println("1");
      
      MsTimer2::set(ledOnDuration + 1, turn_off);
      MsTimer2::start();
    }
    lastDebounceTime = currentTime;
  }
}

void turn_off() {
  analogWrite(PWM_LED_PIN, 0); // Turn off PWM
  MsTimer2::stop();
  isLedActive = false;
  
  // Notify Python GUI
  Serial.println("0");
}