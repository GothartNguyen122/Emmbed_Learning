#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "SECRET.h"
#include "conv1d_model.h"

// === Sensor & Model Setup ===
#define SENSOR_PIN 34
#define DHTTYPE DHT11
#define DPIN 27
Adafruit_BMP280 bmp;
DHT dht(DPIN, DHTTYPE);

const int SEQ_LEN = 60;
float inputBufferA[SEQ_LEN][4];
float inputBufferB[SEQ_LEN][4];
int bufferIndex = 0;
unsigned long lastRead = 0;


float lastVoltage, lastTemp, lastPressure, lastHumidity, lastPrediction;
//Constant of each sensor
const float VOLTAGE_MIN = 0.0, VOLTAGE_MAX = 3.3;
const float TEMP_MIN = 0.0, TEMP_MAX = 85.0;
const float PRESSURE_MIN = 300.0, PRESSURE_MAX = 1100.0;
const float HUMIDITY_MIN = 15.0, HUMIDITY_MAX = 95.0;

float sharedHumidity = NAN;
SemaphoreHandle_t xHumidityMutex;

SemaphoreHandle_t xBufferMutex;
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
TaskHandle_t Task3Handle = NULL;

// === TensorFlow Lite Setup ===
tflite::MicroErrorReporter tflErrorReporter;
tflite::AllOpsResolver tflOpsResolver;
const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;
constexpr int tensorArenaSize = 40 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));


//Chuẩn hóa dữ liệu
float normalize(float value, float minVal, float maxVal) {
  return constrain((value - minVal) / (maxVal - minVal), 0.0, 1.0);
}
//Chuẩn hóa thời gian
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00:00 01/01/1970";
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%H:%M %d/%m/%Y", &timeinfo);
  return String(buffer);
}

void sendData(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://" + String(FIREBASE_HOST) + "/data.json?auth=" + FIREBASE_SECRET;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["time"] = getFormattedTime();
    doc["voltage"] = lastVoltage;
    doc["temperature"] = lastTemp;
    doc["pressure"] = lastPressure;
    doc["humidity"] = lastHumidity;
    doc["prediction"] = lastPrediction;

    String payload;
    serializeJson(doc, payload);

    Serial.println("---- Dữ liệu cảm biến và dự đoán ----");
    Serial.printf("Nhiệt độ: %.2f °C\n", lastTemp);
    Serial.printf("Áp suất: %.2f hPa\n", lastPressure);
    Serial.printf("Độ ẩm: %.2f %%\n", lastHumidity);
    Serial.printf("Điện áp CO: %.2f V\n", lastVoltage);
    Serial.printf("Kết quả dự đoán: %.2f\n", lastPrediction);
    

    int httpResponseCode = http.POST(payload);
    Serial.printf("[SEND] Response: %d\n", httpResponseCode);
    http.end();
  } else {
    Serial.println("[SEND] WiFi not connected");
  }
}

//Task 1: Lấy dữ liệu từ cảm biến ===
void Task1_CollectData(void *pvParameters) {
  while (true) {
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;
    float humidity ;
    if (xSemaphoreTake(xHumidityMutex, portMAX_DELAY) == pdTRUE) {
      humidity = sharedHumidity;
      xSemaphoreGive(xHumidityMutex);
    }
    int sensorValue = analogRead(SENSOR_PIN);
    float voltage = sensorValue * (3.3 / 4095.0);

    if (isnan(humidity) || isnan(temperature) || isnan(pressure) || isnan(voltage)) {
      Serial.println("[ERROR] Một trong các cảm biến không đọc được");
      Serial.printf("Nhiệt độ: %.2f °C\n", temperature);
      Serial.printf("Áp suất: %.2f hPa\n", pressure);
      Serial.printf("Độ ẩm: %.2f %%\n", humidity);
      Serial.printf("Điện áp CO: %.2f V\n", voltage);
      delay(2000);
      continue;
    }

    // Lưu dữ liệu gốc để gửi sau
    lastVoltage = voltage;
    lastTemp = temperature;
    lastPressure = pressure;
    lastHumidity = humidity;

    // Chuẩn hóa
    float voltage_norm = normalize(voltage, VOLTAGE_MIN, VOLTAGE_MAX);
    float temp_norm = normalize(temperature, TEMP_MIN, TEMP_MAX);
    float pressure_norm = normalize(pressure, PRESSURE_MIN, PRESSURE_MAX);
    float humidity_norm = normalize(humidity, HUMIDITY_MIN, HUMIDITY_MAX);

    inputBufferA[bufferIndex][0] = voltage_norm;
    inputBufferA[bufferIndex][1] = temp_norm;
    inputBufferA[bufferIndex][2] = pressure_norm;
    inputBufferA[bufferIndex][3] = humidity_norm;
    bufferIndex++;

    if (bufferIndex >= SEQ_LEN) {
      if (xSemaphoreTake(xBufferMutex, portMAX_DELAY) == pdTRUE) {
        memcpy(inputBufferB, inputBufferA, sizeof(inputBufferA));
        xSemaphoreGive(xBufferMutex);
      }
      bufferIndex = 0;
      xTaskNotifyGive(Task2Handle);
    }
    vTaskDelay(pdMS_TO_TICKS(2000)); // 2s mỗi mẫu
  }
}

// === Task 2: Run Prediction ===
void Task2_Predict(void *pvParameters) {
   while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (xSemaphoreTake(xBufferMutex, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < SEQ_LEN; i++) {
        for (int j = 0; j < 4; j++) {
          tflInputTensor->data.f[i * 4 + j] = inputBufferB[i][j];
        }
      }
      xSemaphoreGive(xBufferMutex);
    }
    TfLiteStatus status = tflInterpreter->Invoke();
    if (status != kTfLiteOk) {
      Serial.println("[PREDICT] Invoke failed");
      lastPrediction = -1;
    } else {
      lastPrediction = tflOutputTensor->data.f[0];
      Serial.printf("[PREDICT] Result: %.4f\n", lastPrediction);
    }
    xTaskNotifyGive(Task3Handle);
  }
}
// Task 3 : Gửi Data
void Task3_Send(void *pvParameters) {
  while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    sendData();
    Serial.println("[SEND] Dữ liệu đã được gửi lên DB");
    Serial.println("-----------------------------------");
  }
}

void setup(){
  Serial.begin(115200);
  delay(1000);
  //Kết nối Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Đã kết nối Wifi");
  //Đồng bộ hóa thời gian
  configTime(7 * 3600, 0, "pool.ntp.org");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) delay(500);
  Serial.println("Đã đồng bộ thời gian");

  // Cảm biến Áp suất & Nhiệt độ
  pinMode(SENSOR_PIN, INPUT);
  // Cảm biến độ ẩm
  dht.begin();
  delay(2000); 
  // Chờ cảm biến ổn định
  float h = dht.readHumidity();

  if (isnan(h)) {
    Serial.println("Không thể khởi tạo DHT11!");
  } else {
    Serial.println("DHT11 đã sẵn sàng!");
    Serial.println(h);
  }

  if (!bmp.begin(0x76)) {
    Serial.println("Không tìm thấy BMP280!");
    while (1);
  }
  //Model
  tflModel = tflite::GetModel(conv1d_model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Sai phiên bản mô hình!");
    while (1);
  }
  //Tensor
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);
  tflInterpreter->AllocateTensors();
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  // Tạo các task
  xBufferMutex = xSemaphoreCreateMutex();
  xHumidityMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(Task1_CollectData, "Task1", 5000, NULL, 1, &Task1Handle, 1);
  xTaskCreatePinnedToCore(Task2_Predict, "Task2", 5000, NULL, 1, &Task2Handle, 1);
  xTaskCreatePinnedToCore(Task3_Send, "Task3", 5000, NULL, 1, &Task3Handle, 1);
}

void loop() {
  if (millis() - lastRead >= 2000) {
    float h = dht.readHumidity();
    if (!isnan(h)) {
      if (xSemaphoreTake(xHumidityMutex, portMAX_DELAY) == pdTRUE) {
        sharedHumidity = h;
        xSemaphoreGive(xHumidityMutex);
      }
    }
    lastRead = millis();
  }
}
