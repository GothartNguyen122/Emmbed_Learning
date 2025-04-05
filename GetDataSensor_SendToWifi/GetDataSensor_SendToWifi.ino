#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>

#define SENSOR_PIN 34
#define DHTTYPE DHT11
#define DPIN 27

const char* ssid = "hello";           // ⚠️ Thay bằng tên WiFi của bạn
const char* password = "1234567899";  // ⚠️ Thay bằng mật khẩu WiFi
const char* serverURL = "http://192.168.140.72:5000/data"; // ⚠️ IP máy tính chạy Flask

Adafruit_BMP280 bmp;
DHT dht(DPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Đã kết nối WiFi!");
  Serial.println(WiFi.localIP());

  pinMode(SENSOR_PIN, INPUT);
  dht.begin();

  if (!bmp.begin(0x76)) {
    Serial.println("❌ Không tìm thấy BMP280!");
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void loop() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float humidity = dht.readHumidity();

  int sensorValue = analogRead(SENSOR_PIN);
  float voltage = sensorValue * (3.3 / 4095.0);

  // In ra Serial
  Serial.println("---- Dữ liệu cảm biến ----");
  Serial.printf("Nhiệt độ: %.2f °C\n", temperature);
  Serial.printf("Áp suất: %.2f hPa\n", pressure);
  Serial.printf("Độ ẩm: %.2f %%\n", humidity);
  Serial.printf("Điện áp CO: %.2f V\n", voltage);
  Serial.println("--------------------------");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // Tạo payload JSON
    String payload = "{";
    payload += "\"voltage\":" + String(voltage, 2) + ",";
    payload += "\"temperature\":" + String(temperature, 2) + ",";
    payload += "\"pressure\":" + String(pressure, 2) + ",";
    payload += "\"humidity\":" + String(humidity, 2);
    payload += "}";

    int httpResponseCode = http.POST(payload);
    Serial.printf("📡 Gửi dữ liệu... Mã phản hồi: %d\n", httpResponseCode);
    http.end();
  } else {
    Serial.println("❌ Không kết nối được WiFi");
  }

  delay(5000);  // Gửi mỗi 5 giây
}
