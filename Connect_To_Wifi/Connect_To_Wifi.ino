#include <WiFi.h>

const char* ssid = "hello";      // Thay thế bằng tên WiFi của bạn
const char* password = "1234567899";  // Thay thế bằng mật khẩu WiFi của bạn

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Đang kết nối đến WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nĐã kết nối WiFi!");
    Serial.print("Địa chỉ IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    Serial.println("WiFi kết nối thành công!");
}
