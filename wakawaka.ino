#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c // initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *ssid = "";
const char *password = "";
const char *apiEndpoint = "https://wakapi.dev/api/compat/wakatime/v1/users//stats/today";
const char *authorizationToken = "";

unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 3 * 60 * 1000;

bool fetchSuccess = false;
String userStats;
String userName;
unsigned long totalSeconds;
unsigned int hours, minutes;
String serverDate;
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};

void secondsToHoursMinutes(unsigned long totalSeconds, unsigned int &hours, unsigned int &minutes)
{
  // Calculate hours and minutes
  hours = totalSeconds / 3600;          // 1 hour = 3600 seconds
  minutes = (totalSeconds % 3600) / 60; // 1 minute = 60 seconds
}

void setup()
{

  Serial.begin(9600);
  connectWiFi();
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  delay(250);                       // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
                                    // display.setContrast (0); // dim display

  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // draw a single pixel
  display.drawPixel(10, 10, SH110X_WHITE);
  // Show the display buffer on the hardware.
  // NOTE: You _must_ call display after making any drawing commands
  // to make them visible on the display hardware!
  display.display();
  delay(2000);
  display.clearDisplay();
  makeGETRequest();

  // text display tests
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Worked For");
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
  display.setTextSize(3);
  display.setTextColor(SH110X_WHITE);
  display.print(14);
  display.display();
  delay(2000);
  if (fetchSuccess == true)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print(hours);
    display.print("hrs");
    display.setCursor(0, 16);
    display.print(minutes);
    display.print("mins");
    display.setCursor(0, 32);
    time_t t = now();
    int currentDay = weekday(t);
    const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    String dayString = days[currentDay - 1];
    display.setTextSize(1);
    display.print(dayString);
    display.setCursor(0, 48);
    display.setTextSize(1);
    display.print(userName);

    display.display();
  }
}

void loop()
{
  unsigned long currentTime = millis();

  // Check if it's time to make a new request
  if (currentTime - lastRequestTime >= requestInterval)
  {
    makeGETRequest();
    lastRequestTime = currentTime;
    if (fetchSuccess == true)
    {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SH110X_BLACK, SH110X_WHITE);
      display.setTextColor(SH110X_WHITE);
      display.setCursor(0, 0);
      display.print(hours);
      display.print("hrs");
      display.setCursor(0, 16);
      display.print(minutes);
      display.print("mins");
      display.setCursor(0, 32);
      time_t t = now();
      int currentDay = weekday(t);
      const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
      String dayString = days[currentDay - 1];
      display.setTextSize(1);
      display.print(dayString);
      display.setCursor(0, 48);
      display.setTextSize(1);
      display.print(userName);

      display.display();
    }
  }
}

void connectWiFi()
{
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi");
}

void makeGETRequest()
{
  HTTPClient http;

  // Your API endpoint URL
  http.begin(apiEndpoint);
  http.addHeader("accept", "application/json");
  http.addHeader("Authorization", authorizationToken);

  http.setTimeout(10000);

  // Send GET request
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String payload = http.getString();

    // Parse JSON response using ArduinoJson
    DynamicJsonDocument jsonDocument(1024); // Adjust the size based on your JSON response
    DeserializationError error = deserializeJson(jsonDocument, payload);

    if (error)
    {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
    }
    else
    {
      // Extract user name from JSON
      userStats = jsonDocument["data"]["human_readable_total"].as<String>();
      totalSeconds = jsonDocument["data"]["total_seconds"].as<int>();
      secondsToHoursMinutes(totalSeconds, hours, minutes);

      serverDate = jsonDocument["data"]["start"].as<String>();
      int year, month, day, hour, minute, second, offsetHour, offsetMinute;
      sscanf(serverDate.c_str(), "%d-%d-%dT%d:%d:%d.%*d%*c%02d:%02d", &year, &month, &day, &hour, &minute, &second, &offsetHour, &offsetMinute);
      int totalOffsetSeconds = (offsetHour * 3600) + (offsetMinute * 60);

      // Set the time using the parsed values
      setTime(hour, minute, second, day, month, year);
      adjustTime(totalOffsetSeconds);
      userStats.remove('i');
      userStats.remove('n');
      userStats.remove('s');
      userName = jsonDocument["data"]["username"].as<String>();
      Serial.print("User stats: ");
      Serial.println(jsonDocument["data"].as<String>());

      // Update boolean indicating fetch success
      fetchSuccess = true;
    }
  }
  else
  {
    Serial.print("HTTP GET request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
