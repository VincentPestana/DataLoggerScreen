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

// Screen type being shown
//  0 - dashboard
//  1 - Humidity averages
//  2 - Temperature averages
//  3 - Gas O2 Averages
int screen;

bool firstMessageShown;

float joyLR;
float joyUD;

float sHumCurr, sTempCurr, sAirCurr;

float sHumLow, sHumHigh, sTempLow, sTempHigh, sAirLow, sAirHigh;

uint16_t dispCounter;

// Keeps a 24 hour history
String history[24];

// Size of the history array
int historyLength = 24;

int currentUptime;

long unsigned lastTimeActive;

int unsigned screensaverWaitTime = 10000;

void setup() {
  Serial.begin(9600);
  // Initialize device.
  dht.begin();
  // Set delay between sensor readings based on sensor details.
  delayMS = 1000;

  // LCD Startup
  lcd.begin(16, 2); //Initialize the 16x2 LCD
  lcd.clear();  //Clear any old data displayed on the LCD

  // Variable initialization
  screen = 0;
  sHumLow = 1000;
  sTempLow = 1000;
  sAirLow = 1000;
  firstMessageShown = false;

  // Welcome message and give gas sensor some time to warm up
  lcd.setCursor(0, 0);
  lcd.print("  Enviro Sense");
  lcd.setCursor(0, 1);
  lcd.print("  Temp Hum Gas");
  delay(10000);

  // Final initializations
  lastTimeActive = millis();
}

void loop() {
  // Delay between measurements.
  delay(delayMS);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  sHumCurr = dht.readHumidity();
  // Read temperature as Celsius (the default)
  sTempCurr = dht.readTemperature();

  // Air sensor
  sAirCurr = analogRead(GasPin);

  // Minimums and maximums
  if (sHumLow > sHumCurr)
    sHumLow = sHumCurr;
  if (sHumHigh < sHumCurr)
    sHumHigh = sHumCurr;

  if (sTempLow > sTempCurr)
    sTempLow = sTempCurr;
  if (sTempHigh < sTempCurr)
    sTempHigh = sTempCurr;

  if (sAirLow > sAirCurr)
    sAirLow = sAirCurr;
  if (sAirHigh < sAirCurr)
    sAirHigh = sAirCurr;
  
  // Controls
  joyLR = map(analogRead(JoyLR), 0, 1006, 0, 10);
  joyUD = map(analogRead(JoyUD), 0, 1006, 0, 10);
  Serial.println(joyLR);
  Serial.println(joyUD);

  // Joystick activity for screensaver
  if (joyLR > 6 || joyLR < 4 || joyUD > 6 || joyUD < 4) {
    lastTimeActive = millis();
  }

  if (joyLR > 6) {
    // Right
    // TODO: Move max into var
    if (screen > 0)
      ChangeScreen(-1);
  } else if (joyLR < 4) {
    // Left
    if (screen < 3)
      ChangeScreen(1);
  }

  Screensaver();

  // Set what info is displayed on lcd
  if (screen == 0)
    DispDashboard();
  else if (screen == 4) {
    DispHitory();
  }
  else {
    DispDetails(screen);
  }

  // Historical saving of data
  //  Every 1 hour (1h*60m*60s*1000ms)
  currentUptime = millis() / 1000 / 60;// / 60;
  if ((currentUptime < historyLength) && (history[currentUptime] == "")) {
    RecordHistory(currentUptime);
  }

  // SerialSensorsOutput();
}

// Show current values and uptime
void DispDashboard() {
  ShowTextMessageOnce("  Dashboard of", "  Information", 2000);

  dispCounter++;

  // LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(sHumCurr, 0);
  lcd.print(" T:");
  lcd.print(sTempCurr, 0);
  lcd.print(" G:");
  lcd.print(sAirCurr, 0);

  // Show uptime on bottom lcd row
  lcd.setCursor(0, 1);
  lcd.print("UT:");
  lcd.print(millis() / 1000);

  // Display information every few iterations
  if (dispCounter > 10) {
    lcd.clear();
    lcd.setCursor(0, 0);

    lcd.print(" Humidity  Temp");
    lcd.setCursor(0, 1);
    lcd.print(" Gas/O2  Sensor");
    delay(3000);
    dispCounter = 0;
  }
}

//  Show averages
void DispDetails(int screenType) {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (screenType) {
    case 1:
      // Humidity
      ShowTextMessageOnce("   Humidity", "", 2000);
      lcd.print("Min   Curr   Max");
      lcd.setCursor(0, 1);
      lcd.print(sHumLow, 0);
      lcd.print("    ");
      lcd.print(sHumCurr, 0);
      lcd.print("    ");
      lcd.print(sHumHigh, 0);
      break;
    case 2:
      // Temperature
      ShowTextMessageOnce("  Temperature", "", 2000);
      lcd.print("Min   Curr   Max");
      lcd.setCursor(0, 1);
      lcd.print(sTempLow, 0);
      lcd.print("    ");
      lcd.print(sTempCurr, 0);
      lcd.print("    ");
      lcd.print(sTempHigh, 0);
      break;
    case 3:
      // Air
      ShowTextMessageOnce(" O2 Gas Sensor", "", 2000);
      lcd.print("Min   Curr   Max");
      lcd.setCursor(0, 1);
      lcd.print(sAirLow, 0);
      lcd.print("   ");
      lcd.print(sAirCurr, 0);
      lcd.print("   ");
      lcd.print(sAirHigh, 0);
      break;
    default:
      DispDashboard();
      break;
  }
  
}

// History screen, scrollable
void DispHitory() {
  lcd.clear();
  lcd.setCursor(0, 0);
  ShowTextMessageOnce("  Historical  ", "", 2000);
  // No history
  if (history[0] == "") {
    lcd.setCursor(0, 0);
    lcd.print("  Historical  ");
    lcd.setCursor(0, 0);
    lcd.print("No previous data");
  } else {
    
  }


}

void ChangeScreen(int changeValue) {
  screen = screen + changeValue;
  firstMessageShown = false;
}

// Show a message on the LCD for a amount of time
void ShowTextMessageOnce(String topLine, String bottomLine, int showTime) {
  if (firstMessageShown)
    return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(topLine);
  lcd.setCursor(0, 1);
  lcd.print(bottomLine);
  delay(showTime);
  firstMessageShown = true;

  // Clean up
  lcd.clear();
  lcd.setCursor(0, 0);
}

// Records current values to array at current time slot
void RecordHistory(int historyIndex) {
  // "[time ago],[humidity],[temperature],[gas]"
  String dataToSave = String(historyIndex) + "," + String(sHumCurr, 0) + "," + String(sTempCurr, 0) + "," + String(sAirCurr, 0);
  history[historyIndex] = dataToSave;
}

void Screensaver() {
  if ((millis() - lastTimeActive) > screensaverWaitTime) {
    lcd.noDisplay();
  } else {
    lcd.display();
  }
}

// Prints sensor values out to Serial, if available
void SerialSensorsOutput() {
  // Check if Serial connection is available
  if (!Serial)
    return;

  // Print to serial
  Serial.print("Temp: ");
  Serial.print(sTempCurr, 0);
  Serial.print("\tHumidity: ");
  Serial.print(sHumCurr, 0);
  Serial.print("\tGas: ");
  Serial.print(sAirCurr, 0);
  
  Serial.print("\t: ");
  Serial.print(joyLR, 0);
  Serial.print("\t: ");
  Serial.print(joyUD, 0);
  
  Serial.println(" ");
}