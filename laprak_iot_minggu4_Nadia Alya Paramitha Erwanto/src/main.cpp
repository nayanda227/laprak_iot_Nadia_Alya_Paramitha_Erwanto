#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Setup
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// OpenWeatherMap
String apiKey = "6375be75703e403d828c83238072ae30";
String city = "Malang";
String units = "metric";
String server = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&appid=" + apiKey;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT Sensor
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LED Pins
#define LED_HOT 2
#define LED_COLD 4
#define LED_NORMAL 5

// Buzzer Pin
#define BUZZER_PIN 18

void setup() {
  Serial.begin(115200);

  // Start OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(1000);
  display.clearDisplay();
  
  // Start DHT
  dht.begin();

  // LED and Buzzer setup
  pinMode(LED_HOT, OUTPUT);
  pinMode(LED_COLD, OUTPUT);
  pinMode(LED_NORMAL, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Connect WiFi
  WiFi.begin(ssid, password);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Connecting WiFi...");
  display.display();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("WiFi Connected!");
  display.display();
  delay(1000);
}

void loop() {
  display.clearDisplay();
  
  // DHT Local Sensor Read
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Check WiFi
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(server);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);

      // Parsing JSON
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        String cityName = doc["name"];
        String country = doc["sys"]["country"];
        String weatherMain = doc["weather"][0]["main"];
        String weatherDesc = doc["weather"][0]["description"];
        float temp = doc["main"]["temp"];
        float feelsLike = doc["main"]["feels_like"];
        float tempMin = doc["main"]["temp_min"];
        float tempMax = doc["main"]["temp_max"];
        int pressure = doc["main"]["pressure"];
        int humidity = doc["main"]["humidity"];
        float windSpeed = doc["wind"]["speed"];
        int windDeg = doc["wind"]["deg"];
        long sunrise = doc["sys"]["sunrise"];
        long sunset = doc["sys"]["sunset"];

        // Print ke Serial Monitor
        Serial.println("========== Weather Info ==========");
        Serial.println("City         : " + cityName);
        Serial.println("Country      : " + country);
        Serial.println("Weather      : " + weatherMain);
        Serial.println("Description  : " + weatherDesc);
        Serial.print("Temperature  : "); Serial.print(temp); Serial.println(" °C");
        Serial.print("Feels Like   : "); Serial.print(feelsLike); Serial.println(" °C");
        Serial.print("Min Temp     : "); Serial.print(tempMin); Serial.println(" °C");
        Serial.print("Max Temp     : "); Serial.print(tempMax); Serial.println(" °C");
        Serial.print("Pressure     : "); Serial.print(pressure); Serial.println(" hPa");
        Serial.print("Humidity     : "); Serial.print(humidity); Serial.println(" %");
        Serial.print("Wind Speed   : "); Serial.print(windSpeed); Serial.println(" m/s");
        Serial.print("Wind Degree  : "); Serial.print(windDeg); Serial.println(" °");

        // Konversi sunrise & sunset dari epoch ke jam:menit:detik
        time_t rawSunrise = sunrise;
        struct tm * sunriseTime = gmtime(&rawSunrise);
        Serial.print("Sunrise      : ");
        Serial.printf("%02d:%02d:%02d\n", sunriseTime->tm_hour, sunriseTime->tm_min, sunriseTime->tm_sec);

        time_t rawSunset = sunset;
        struct tm * sunsetTime = gmtime(&rawSunset);
        Serial.print("Sunset       : ");
        Serial.printf("%02d:%02d:%02d\n", sunsetTime->tm_hour, sunsetTime->tm_min, sunsetTime->tm_sec);

        Serial.println("===================================");

        // OLED Tampilan sederhana
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("City:");
        display.print(cityName);
        display.setCursor(0,10);
        display.print("Temp:");
        display.print(temp);
        display.print("C");
        display.setCursor(0,20);
        display.print("Humid:");
        display.print(humidity);
        display.print("%");
        display.setCursor(0,30);
        display.print("Weather:");
        display.print(weatherMain);
        display.display();

        // LED & Buzzer Logic
        digitalWrite(LED_HOT, LOW);
        digitalWrite(LED_COLD, LOW);
        digitalWrite(LED_NORMAL, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        
        if (temp >= 30.0) {
          digitalWrite(LED_HOT, HIGH);
        } else if (temp <= 20.0) {
          digitalWrite(LED_COLD, HIGH);
        } else {
          digitalWrite(LED_NORMAL, HIGH);
        }
        
        if (weatherDesc.indexOf("rain") >= 0 || weatherDesc.indexOf("storm") >= 0) {
          tone(BUZZER_PIN, 1000);
          delay(500);
          noTone(BUZZER_PIN);
        }
        
      } else {
        Serial.println("Failed to parse JSON");
      }

    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  }
  
  delay(60000); // Update per 1 menit
}
