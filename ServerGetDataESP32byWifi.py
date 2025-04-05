from flask import Flask, request, jsonify
import csv
import os

app = Flask(__name__)
csv_filename = 'sensor_data.csv'

# Ghi tiêu đề nếu file chưa có
if not os.path.exists(csv_filename):
    with open(csv_filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Voltage (V)", "Temperature (°C)", "Pressure (hPa)", "Humidity (%)"])

@app.route('/data', methods=['POST'])
def receive_data():
    try:
        data = request.get_json()
        with open(csv_filename, mode='a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([
                data.get("voltage"),
                data.get("temperature"),
                data.get("pressure"),
                data.get("humidity")
            ])
        print("✔️ Nhận được:", data)
        return jsonify({"status": "success"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
