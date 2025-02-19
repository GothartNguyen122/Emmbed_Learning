#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include<DHT.h>

#define SENSOR_PIN 34  // GPIO34 là chân ADC của ESP32
#define DHTTYPE DHT11
#define DPIN 27

// Tạo đối tượng cảm biến BMP280
Adafruit_BMP280 bmp;
DHT dht(DPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);  // Khởi tạo Serial Monitor
  //Start CO 
  pinMode(SENSOR_PIN, INPUT);  // Cấu hình chân cảm biến là đầu vào
  dht.begin();
  // Khởi động BMP280
  if (!bmp.begin(0x76)) {  // Địa chỉ I2C mặc định của BMP280 là 0x76
    Serial.println("Không tìm thấy BMP280! Vui lòng kiểm tra kết nối.");
    while (1);  // Dừng chương trình nếu không tìm thấy cảm biến
  }

  // Cấu hình BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,      
                  Adafruit_BMP280::SAMPLING_X2,      
                  Adafruit_BMP280::SAMPLING_X16,     
                  Adafruit_BMP280::FILTER_X16,       
                  Adafruit_BMP280::STANDBY_MS_500);  
}

void loop() {
  // Đọc dữ liệu từ cảm biến
  float temperature = bmp.readTemperature();  
  float pressure = bmp.readPressure() / 100.0F;  
  float altitude = bmp.readAltitude(1013.25);
  float humidity = dht.readHumidity();
  //Hiển thị dữ liệu đọc khí CO
  int sensorValue = analogRead(SENSOR_PIN); 
  float voltage = sensorValue * (3.3 / 4095.0); 

  // Hiển thị giá trị trên Serial Monitor
  Serial.print("Raw Value: ");
  Serial.print(sensorValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage);
  Serial.println(" V");

  // Hiển thị dữ liệu lên Serial Monitor
  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Độ ẩm: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Áp suất: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  // Serial.print("Độ cao: ");    
  // Serial.print(altitude);
  // Serial.println(" m");

  Serial.println("--------------------------");
  delay(5000);  // Chờ 1 giây trước khi đọc lại
}
