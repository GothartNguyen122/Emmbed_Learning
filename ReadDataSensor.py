import serial
import csv
import time
import os

# Cấu hình Serial
SERIAL_PORT = 'COM5'
BAUD_RATE = 115200

# Tên file CSV
csv_filename = 'sensor_data.csv'

# Kiểm tra nếu file chưa tồn tại thì ghi tiêu đề
if not os.path.exists(csv_filename):
    with open(csv_filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Voltage (V)", "Temperature (°C)", "Pressure (hPa)", "Humidity (%)"])

# Mở cổng Serial
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

# Chờ 5 giây trước khi bắt đầu vòng lặp
time.sleep(5)

try:
    with open(csv_filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        while True:
            try:
                line = ser.readline().decode('utf-8').strip()
                
                if line:
                    print(line)  # Hiển thị dữ liệu nhận được
                    
                    if "Voltage:" in line:
                        voltage = float(line.split("Voltage:")[1].strip().split(" ")[0])
                    elif "Nhiệt độ:" in line:
                        temperature = float(line.split("Nhiệt độ:")[1].strip().split(" ")[0])
                    elif "Áp suất:" in line:
                        pressure = float(line.split("Áp suất:")[1].strip().split(" ")[0])
                    elif "Độ ẩm:" in line:
                        humidity = float(line.split("Độ ẩm:")[1].strip().split(" ")[0])
                        
                        if None not in (voltage, temperature, pressure, humidity):
                            writer.writerow([voltage, temperature, pressure, humidity])
                            file.flush()  # Đảm bảo dữ liệu được ghi ngay vào file
                            print("Dữ liệu đã ghi vào file CSV.")
            except Exception as e:
                print(f"Lỗi khi xử lý dòng: {line}. Bỏ qua. Lỗi: {e}")
                continue
except KeyboardInterrupt:
    print("Dừng chương trình.")
    ser.close()
