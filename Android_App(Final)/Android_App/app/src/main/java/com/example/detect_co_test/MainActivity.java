package com.example.detect_co_test;

import android.graphics.Color;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.TypedValue;
import android.view.Gravity;
import android.widget.ImageView;
import android.widget.LinearLayout;
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
                        Object predictionObj = latestEntry.child("prediction").getValue();

                        // Chuyển dữ liệu sang String để tránh lỗi
                        String voltage = voltageObj != null ? voltageObj.toString() : "N/A";
                        String temperature = temperatureObj != null ? temperatureObj.toString() : "N/A";
                        String pressure = pressureObj != null ? pressureObj.toString() : "N/A";
                        String humidity = humidityObj != null ? humidityObj.toString() : "N/A";
                        String prediction = predictionObj != null ? predictionObj.toString() : "N/A";

                        // Cập nhật UI
                        voltageText.setText(voltage + "V");
                        temperatureText.setText(temperature + "°C");
                        pressureText.setText(pressure + " hPa");
                        humidityText.setText(humidity + "%");

                        // 👇 Thay đổi giao diện theo prediction
                        LinearLayout mainLayout = findViewById(R.id.main);
                        TextView dangerText = findViewById(R.id.danger_text);
                        ImageView warningImage = findViewById(R.id.warning_image);

                        if (prediction.equals("1")) {
//                            mainLayout.setBackgroundResource(R.drawable.back_danger);
                            mainLayout.setBackgroundColor(Color.parseColor("#CC5500"));
                            dangerText.setText("Nguy hiểm - Có cháy!");
                            warningImage.setImageResource(R.drawable.icon_danger);
                        } else {
                            mainLayout.setBackgroundResource(R.drawable.back_safe);
//                            mainLayout.setBackgroundColor(Color.parseColor("#2E7D32"));
                            dangerText.setText("An toàn");
                            warningImage.setImageResource(R.drawable.icon_safee);
                        }
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

//                    while (historyTable.getChildCount() > 1) {
//                        historyTable.removeViewAt(1);
//                    }
                    historyTable.removeAllViews(); // Xóa hết nội dung cũ để tạo lại hoàn toàn

// Thêm dòng tiêu đề bảng
                    TableRow headerRow = new TableRow(MainActivity.this);
                    headerRow.setBackgroundColor(getResources().getColor(android.R.color.darker_gray));

                    headerRow.addView(createHeaderTextView("Thời gian"));
                    headerRow.addView(createHeaderTextView("CO"));
                    headerRow.addView(createHeaderTextView("Nhiệt độ"));
                    headerRow.addView(createHeaderTextView("Áp suất"));
                    headerRow.addView(createHeaderTextView("Độ ẩm"));

                    historyTable.addView(headerRow);


                    for (DataSnapshot entry : dataSnapshot.getChildren()) {
                        String time = entry.child("time").getValue() != null ? entry.child("time").getValue().toString() : "N/A";
                        String voltage = entry.child("voltage").getValue() != null ? entry.child("voltage").getValue().toString() + "V" : "N/A";
                        String temperature = entry.child("temperature").getValue() != null ? entry.child("temperature").getValue().toString() + "°C" : "N/A";
                        String pressure = entry.child("pressure").getValue() != null ? entry.child("pressure").getValue().toString() + " hPa" : "N/A";
                        String humidity = entry.child("humidity").getValue() != null ? entry.child("humidity").getValue().toString() + "%" : "N/A";

                        TableRow row = new TableRow(MainActivity.this);

                        TextView timeTextView = createTextView(time);
                        String[] timeParts = time.split(" "); // Tách "04:52" và "09/04/2025"
                        if (timeParts.length == 2) {
                            String timeOnly = timeParts[0]; // "04:52"
                            String[] dateParts = timeParts[1].split("/"); // ["09", "04", "2025"]

                            if (dateParts.length == 3) {
                                String newTime = timeOnly + "  " + dateParts[0] + "/" + dateParts[1]; // "04:52  09/04"
                                timeTextView.setText(newTime);
                            } else {
                                timeTextView.setText(time); // fallback nếu không đúng định dạng
                            }
                        } else {
                            timeTextView.setText(time); // fallback
                        }

//                        String[] timeParts = time.split(" ");
//                        if (timeParts.length == 2) {
//                            timeTextView.setText(timeParts[0] +  timeParts[1]);
//                            timeTextView.setMaxLines(2); // cho phép xuống dòng
//                        }

                        row.addView(timeTextView);
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
    // Trong hàm tạo TextView cho từng cột
    private TextView createHeaderTextView(String text) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(getResources().getColor(android.R.color.white));
        textView.setPadding(8, 8, 8, 8);
        textView.setGravity(Gravity.CENTER);
        textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 14);
        textView.setTypeface(null, android.graphics.Typeface.BOLD);
        textView.setBackgroundResource(R.drawable.cell_border);
        textView.setMinHeight(dpToPx(48)); // Chiều cao dòng tiêu đề

        TableRow.LayoutParams params = new TableRow.LayoutParams(
                TableRow.LayoutParams.WRAP_CONTENT,
                TableRow.LayoutParams.WRAP_CONTENT,
                1f
        );
        textView.setLayoutParams(params);

        return textView;
    }

    private TextView createTextView(String text) {
        TextView textView = new TextView(this);
        textView.setText(text);
        textView.setTextColor(getResources().getColor(android.R.color.white));
        textView.setPadding(8, 8, 8, 8);
        textView.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL); // Căn giữa cả dọc lẫn ngang
        textView.setMaxLines(2); // Cẩn thận giới hạn số dòng
        textView.setEllipsize(TextUtils.TruncateAt.END);

        TableRow.LayoutParams params = new TableRow.LayoutParams(
                TableRow.LayoutParams.WRAP_CONTENT,
                TableRow.LayoutParams.WRAP_CONTENT
        );
        textView.setLayoutParams(params);
        textView.setBackgroundResource(R.drawable.cell_border);

        // Set chiều cao tối thiểu (đảm bảo khớp với ô "Thời gian" có 2 dòng)
        textView.setMinHeight(dpToPx(70)); // bạn có thể điều chỉnh nếu thấy cao quá hoặc thấp quá

        return textView;
    }

    // Hàm convert dp -> px
    private int dpToPx(int dp) {
        float density = getResources().getDisplayMetrics().density;
        return Math.round((float) dp * density);
    }



    // Hàm chuyển timestamp thành định dạng giờ:phút:giây
//    private String formatTimestamp(Long timestamp) {
////        if (timestamp == null) return "N/A";
////        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss", Locale.getDefault());
////        return sdf.format(new Date(timestamp * 1000));
//    }

}
