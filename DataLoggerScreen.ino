// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <LiquidCrystal.h>

// Define pins
#define DHTPIN 2
#define GasPin 4
#define JoyLR 1
#define JoyUD 0

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

uint32_t delayMS;

void setup() {
  Serial.begin(9600);
  // Initialize device.
  dht.begin();
  // Set delay between sensor readings based on sensor details.
  delayMS = 1000;

  // LCD Startup
  lcd.begin(16, 2); //Initialize the 16x2 LCD
  lcd.clear();  //Clear any old data displayed on the LCD
}

void loop() {
  // Delay between measurements.
  delay(delayMS);
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float sHumidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float sTemp = dht.readTemperature();

  // Air sensor
  float sAir = analogRead(GasPin);
  // Controls
  float joyLR = analogRead(JoyLR);
  float joyUD = analogRead(JoyUD);

//  Serial.print("\t: ");
//  Serial.print(joyLR, 0);
//  Serial.print("\t: ");
//  Serial.print(joyUD, 0);
  // LCD Display
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(sHumidity, 0);
  lcd.print(" T:");
  lcd.print(sTemp, 0);
  lcd.print(" G:");
  lcd.print(sAir, 0);

  // Show uptime on bottom lcd row
  lcd.setCursor(0, 1);
  lcd.print("UT:");
  lcd.print(millis() / 1000);

  // Print to serial
  Serial.print("Temp: ");
  Serial.print(sTemp, 0);
  Serial.print("\tHumidity: ");
  Serial.print(sHumidity, 0);
  Serial.print("\tGas: ");
  Serial.println(sAir, 0);
}
