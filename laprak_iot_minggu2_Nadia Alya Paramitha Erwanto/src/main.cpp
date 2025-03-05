#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Pin Konfigurasi
#define DHTPIN 4         // Pin untuk DHT22
#define DHTTYPE DHT22    // Menggunakan DHT22
#define LDRPIN 34        // Pin untuk LDR (ESP32)

// Inisialisasi Sensor & OLED
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
    Serial.begin(115200);
    dht.begin();

    // Inisialisasi OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 gagal ditemukan!"));
        for (;;); // Berhenti jika gagal
    }

    display.clearDisplay();
    display.display();

    Serial.println("Sistem Monitoring Sensor Dimulai...");
}

void loop() {
    // Membaca suhu & kelembapan dari DHT22
    float suhu = dht.readTemperature();
    float kelembapan = dht.readHumidity();
    
    // Membaca intensitas cahaya dari LDR
    int intensitasCahaya = analogRead(LDRPIN);
    float cahayaPersen = map(intensitasCahaya, 0, 4095, 0, 100); // Konversi ke %

    // Cek apakah sensor DHT berhasil terbaca
    if (isnan(suhu) || isnan(kelembapan)) {
        Serial.println("❌ Gagal membaca dari DHT sensor!");
        return;
    }

    // Menampilkan di Serial Monitor
    Serial.print("🌡️ Suhu: "); Serial.print(suhu); Serial.println(" °C");
    Serial.print("💧 Kelembapan: "); Serial.print(kelembapan); Serial.println(" %");
    Serial.print("☀️ Cahaya: "); Serial.print(cahayaPersen); Serial.println(" %");
    Serial.println("---------------------------");

    // Menampilkan di OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    display.setCursor(0, 0);
    display.println("Monitoring Sensor");

    display.setCursor(0, 16);
    display.print("Suhu: "); display.print(suhu); display.println(" C");

    display.setCursor(0, 32);
    display.print("Kelembapan: "); display.print(kelembapan); display.println(" %");

    display.setCursor(0, 48);
    display.print("Cahaya: "); display.print(cahayaPersen); display.println(" %");

    display.display();

    delay(2000); // Update setiap 2 detik
}
