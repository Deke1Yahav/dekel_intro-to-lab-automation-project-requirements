// This example demonstrates continuous conversion mode using the
// DRDY pin to check for conversion completion.

#include <Adafruit_MAX31856.h>

#define DRDY_PIN 5
const byte rotaryPin = A0;
const byte switch_wolt = 10;

int k1 = 1
int setpoint = 40;
int measured_temp = 25 ;

// int integral = 0;

int calc_error(int setpoint, int measured_temp) {
  int error = setpoint - measured_temp;
  return error;
}

// Use software SPI: CS, DI, DO, CLK
//Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(10, 11, 12, 13);
// use hardware SPI, just pass in the CS pin
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856(4);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("MAX31856 thermocouple test");

  pinMode(DRDY_PIN, INPUT);

  if (!maxthermo.begin()) {
    Serial.println("Could not initialize thermocouple.");
    while (1) delay(10);

    pinMode(switch_wolt, OUTPUT);

    // pin10 is wired pwm
    // Keep it as an input so it doesn't fight pin3's output.
  }


  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);

  Serial.print("Thermocouple type: ");


  switch (maxthermo.getThermocoupleType()) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x32 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }

  maxthermo.setConversionMode(MAX31856_CONTINUOUS);
}



void loop() {
  int activation = k1 * calc_error(setpoint, measured_temp);
  // int rotaryValue = analogRead(rotaryPin);
  // byte dutyCycle = map(rotaryValue, 0, 1023, 0, 255);
  byte activation_power = map(rotaryValue, 0, 100, 0, 255); 
  analogWrite(switch_wolt, activation_power);

  Serial.println(activation);
  delay(100);


  // The DRDY output goes low when a new conversion result is available
  int count = 0;
  while (digitalRead(DRDY_PIN)) {
    if (count++ > 200) {
      count = 0;
      Serial.print(".");
    }
  }
  measured_temp = maxthermo.readThermocoupleTemperature();
  Serial.println(maxthermo.readThermocoupleTemperature());
}