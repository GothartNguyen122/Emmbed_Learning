package com.example.detect_co_test;

import android.os.Bundle;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {

    private TextView voltageText, temperatureText, pressureText, humidityText;
    private DatabaseReference ref;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Ánh xạ TextView từ giao diện
        voltageText = findViewById(R.id.vol_value);
        temperatureText = findViewById(R.id.temperature_value);
        pressureText = findViewById(R.id.pressure_value);
        humidityText = findViewById(R.id.humidity_value);
        this.ref = FirebaseDatabase.getInstance().getReference("data");
        getDataforView2();
        loadHistoryData();
    }
    private void getDataforView2(){
        // Kết nối Firebase Database
        // Lấy tối đa 50 dòng, chỉ hiển thị dòng dữ liệu mới nhất
        this.ref.orderByKey().limitToLast(50).addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                if (dataSnapshot.exists()) {
                    DataSnapshot latestEntry = null;
                    for (DataSnapshot entry : dataSnapshot.getChildren()) {
                        latestEntry = entry; // Duyệt đến phần tử cuối cùng (dữ liệu mới nhất)
                    }

                    if (latestEntry != null) {
                        // Đọc dữ liệu từ Firebase
                        Object voltageObj = latestEntry.child("voltage").getValue();
                        Object temperatureObj = latestEntry.child("temperature").getValue();
                        Object pressureObj = latestEntry.child("pressure").getValue();
                        Object humidityObj = latestEntry.child("humidity").getValue();

                        // Chuyển dữ liệu sang String để tránh lỗi
                        String voltage = voltageObj != null ? voltageObj.toString() : "N/A";
                        String temperature = temperatureObj != null ? temperatureObj.toString() : "N/A";
                        String pressure = pressureObj != null ? pressureObj.toString() : "N/A";
                        String humidity = humidityObj != null ? humidityObj.toString() : "N/A";

                        // Cập nhật UI
                        voltageText.setText("Voltage: " + voltage + "V");
                        temperatureText.setText("Temperature: " + temperature + "°C");
                        pressureText.setText("Pressure: " + pressure + " hPa");
                        humidityText.setText("Humidity: " + humidity + "%");
                    }
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {
                voltageText.setText("Lỗi tải dữ liệu!");
                temperatureText.setText("Lỗi tải dữ liệu!");
                pressureText.setText("Lỗi tải dữ liệu!");
                humidityText.setText("Lỗi tải dữ liệu!");
            }
        });
    }

    private void loadHistoryData() {
        this.ref.orderByKey().limitToLast(100).addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                if (dataSnapshot.exists()) {
                    TableLayout historyTable = findViewById(R.id.history_table);

                    while (historyTable.getChildCount() > 1) {
                        historyTable.removeViewAt(1);
                    }

                    for (DataSnapshot entry : dataSnapshot.getChildren()) {
                        String time = entry.child("time").getValue() != null ? entry.child("time").getValue().toString() : "N/A";
                        String voltage = entry.child("voltage").getValue() != null ? entry.child("voltage").getValue().toString() + "V" : "N/A";
                        String temperature = entry.child("temperature").getValue() != null ? entry.child("temperature").getValue().toString() + "°C" : "N/A";
                        String pressure = entry.child("pressure").getValue() != null ? entry.child("pressure").getValue().toString() + " hPa" : "N/A";
                        String humidity = entry.child("humidity").getValue() != null ? entry.child("humidity").getValue().toString() + "%" : "N/A";

                        TableRow row = new TableRow(MainActivity.this);
                        row.addView(createTextView(time));
                        row.addView(createTextView(voltage));
                        row.addView(createTextView(temperature));
                        row.addView(createTextView(pressure));
                        row.addView(createTextView(humidity));

                        historyTable.addView(row);
                    }
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {
            }
        });
    }

    // Hàm tạo TextView cho từng cột
    private TextView createTextView(String text) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(getResources().getColor(android.R.color.white));
        textView.setPadding(8, 8, 8, 8);
        return textView;
    }

    // Hàm chuyển timestamp thành định dạng giờ:phút:giây
//    private String formatTimestamp(Long timestamp) {
////        if (timestamp == null) return "N/A";
////        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss", Locale.getDefault());
////        return sdf.format(new Date(timestamp * 1000));
//    }

}
