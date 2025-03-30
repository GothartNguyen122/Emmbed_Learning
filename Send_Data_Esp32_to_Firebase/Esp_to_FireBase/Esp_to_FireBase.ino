#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ctime>

// Thông tin kết nối WiFi
#define WIFI_SSID "mynetwork"
#define WIFI_PASSWORD "12345678"

// Thông tin Firebase
#define FIREBASE_HOST "detectcohumidpressure-default-rtdb.firebaseio.com"
#define FIREBASE_SECRET "WeYizQ3oQTxSfdaTbTdx1Lx07QwMHvOD7IpS7Wl3"  // Lấy từ Firebase settings


void setup() {
    Serial.begin(115200);
    
    // Kết nối WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to Wi-Fi");
}

void sendData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = "https://" + String(FIREBASE_HOST) + "/data.json?auth=" + FIREBASE_SECRET;
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        
        // Tạo dữ liệu JSON
        StaticJsonDocument<200> doc;
        doc["time"] = (unsigned long)time(nullptr);
        doc["voltage"] = random(220, 230) / 10.0;
        doc["temperature"] = random(20, 35);
        doc["pressure"] = random(950, 1050);
        doc["humidity"] = random(30, 70);
        doc["prediction"] = random(0, 2);  // Giả định dự đoán là 0 hoặc 1
        
        String payload;
        serializeJson(doc, payload);
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode > 0) {
            Serial.println("Data sent successfully: " + String(httpResponseCode));
        } else {
            Serial.println("Error sending data: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("Wi-Fi not connected");
    }
}

void loop() {
    sendData();  // Gửi dữ liệu lên Firebase
    delay(5000); // Chờ 5 giây trước khi gửi dữ liệu tiếp theo
}

