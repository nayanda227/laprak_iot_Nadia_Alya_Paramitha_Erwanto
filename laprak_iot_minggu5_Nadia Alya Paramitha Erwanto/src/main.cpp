#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int LED_RED = 2;
const int DHT_PIN = 15;
const int BUTTON_PIN = 4;

DHTesp dht;

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastButtonDisplay = 0;
bool lastButtonState = HIGH;
bool showButtonMessage = false;

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  if ((char)payload[0] == '1') {
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(LED_RED, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.publish("IOT/Test1/mqtt", "Test IOT");
      client.subscribe("IOT/Test1/mqtt");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.setup(DHT_PIN, DHTesp::DHT22);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal dimulai"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();
  delay(1000);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();

  // Baca tombol
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("Tombol ditekan!");
    client.publish("IOT/Test1/button", "Tombol ditekan!");
    digitalWrite(LED_RED, !digitalRead(LED_RED)); // Toggle LED

    showButtonMessage = true;
    lastButtonDisplay = now;
  }
  lastButtonState = buttonState;

  // Baca suhu dan kelembaban tiap 2 detik
  if (now - lastMsg > 2000) {
    lastMsg = now;
    TempAndHumidity data = dht.getTempAndHumidity();

    String tempStr = String(data.temperature, 2);
    String humStr = String(data.humidity, 1);

    client.publish("IOT/Test1/temp", tempStr.c_str());
    client.publish("IOT/Test1/hum", humStr.c_str());

    Serial.println("Temperature: " + tempStr);
    Serial.println("Humidity: " + humStr);

    // Tampilkan ke OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(tempStr);
    display.println(" C");
    display.print("Hum : ");
    display.print(humStr);
    display.println(" %");

    if (showButtonMessage) {
      display.setCursor(0, 40);
      display.println("Button Pressed!");
    }

    display.display();
  }

  // Hilangkan tulisan tombol setelah 2 detik
  if (showButtonMessage && (now - lastButtonDisplay > 2000)) {
    showButtonMessage = false;
  }
}
