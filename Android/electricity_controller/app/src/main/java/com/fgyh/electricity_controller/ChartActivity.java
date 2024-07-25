package com.fgyh.electricity_controller;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class ChartActivity extends AppCompatActivity {

    private RelativeLayout frown;
    private TextView primaryFon;
    private TextView someId;
    private LinearLayout btnChart;
    private LinearLayout btnSettings;
    private LinearLayout btnPic;
    private RelativeLayout genericAva;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chart);

        // Initialize views
        primaryFon = findViewById(R.id.primary_fon);
        frown = findViewById(R.id.frown);
        someId = findViewById(R.id.some_id);
        genericAva = findViewById(R.id.generic_ava);
        btnChart = findViewById(R.id.btn_chart);
        btnSettings = findViewById(R.id.btn_settings);
        btnPic = findViewById(R.id.btn_pic);

        // Set click listeners for bottom navigation buttons
        btnChart.setOnClickListener(v -> {
            Intent intent = new Intent(ChartActivity.this, MainActivity.class);
            startActivity(intent);
        });

        btnSettings.setOnClickListener(v -> {
            Intent intent = new Intent(ChartActivity.this, SettingsActivity.class);
            startActivity(intent);
        });

        btnPic.setOnClickListener(v -> {
            Intent intent = new Intent(ChartActivity.this, ChartActivity.class);
            startActivity(intent);
        });

        // Set click listener for avatar to show confirmation dialog
        genericAva.setOnClickListener(v -> {
            new AlertDialog.Builder(ChartActivity.this)
                    .setTitle("确认退出")
                    .setMessage("您确定要退出吗？")
                    .setPositiveButton("确认", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            finish(); // 关闭当前Activity
                        }
                    })
                    .setNegativeButton("取消", null)
                    .show();
        });
    }
}
