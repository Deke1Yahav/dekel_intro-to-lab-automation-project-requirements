#include <MsTimer2.h>

// Pin definitions
const byte BUZZER_PIN = 5;   // Buzzer connected to pin D5
const byte MONITOR_PIN = 4;  // Pin D4 configured as INPUT to monitor D5
const byte BUTTON_PIN = 2;   // Button bridged to pin D2

// Global variables
volatile unsigned long ledOnDuration = 1000; 
volatile byte ledBrightness = 255;           
volatile bool isLedActive = false;           

// Debouncing variables
volatile unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// Function declarations
void buttonInterrupt();
void turn_off();
void parseAndPlayMelody(String data);

void setup() {
  Serial.begin(9600);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  pinMode(MONITOR_PIN, INPUT);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, FALLING);
}

void loop() {
  if (Serial.available() > 0) {
    String incomingString = Serial.readStringUntil('\n');
    incomingString.trim();
    
    // Check if it's a melody command from Python (Starts with "M:")
    if (incomingString.startsWith("M:")) {
      Serial.println("Playing custom melody from PC...");
      parseAndPlayMelody(incomingString.substring(2)); // Pass data after "M:"
      Serial.println("Melody Finished!");
    } 
    // Standard duration/brightness command
    else {
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
          
          if (isLedActive) {
            analogWrite(BUZZER_PIN, ledBrightness);
          }
          
          Serial.print("I received: ");
          Serial.print(ledOnDuration);
          Serial.print(" ms with volume: ");
          Serial.println(ledBrightness);
        } else {
          Serial.println("Error: Invalid parameters.");
        }
      }
    }
  }
}

// Parses "freq,duration,freq,duration..." and plays it
void parseAndPlayMelody(String data) {
  int length = data.length();
  int lastIndex = 0;
  
  while (lastIndex < length) {
    // Find next comma for frequency
    int nextComma = data.indexOf(',', lastIndex);
    if (nextComma == -1) break; // Expecting pairs
    
    int freq = data.substring(lastIndex, nextComma).toInt();
    lastIndex = nextComma + 1;
    
    // Find next comma or end of string for duration
    nextComma = data.indexOf(',', lastIndex);
    int dur;
    if (nextComma == -1) {
      dur = data.substring(lastIndex).toInt();
      lastIndex = length; // Reach the end
    } else {
      dur = data.substring(lastIndex, nextComma).toInt();
      lastIndex = nextComma + 1;
    }
    
    // Play the note if valid
    if (freq > 0 && dur > 0) {
      tone(BUZZER_PIN, freq, dur);
      delay(dur * 1.30); // Pause between notes
      noTone(BUZZER_PIN);
    }
  }
}

void buttonInterrupt() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    if (!isLedActive) {
      isLedActive = true;
      analogWrite(BUZZER_PIN, ledBrightness);
      Serial.println("1");
      
      MsTimer2::set(ledOnDuration + 1, turn_off);
      MsTimer2::start();
    }
    lastDebounceTime = currentTime;
  }
}

void turn_off() {
  analogWrite(BUZZER_PIN, 0); 
  MsTimer2::stop();
  isLedActive = false;
  Serial.println("0");
}