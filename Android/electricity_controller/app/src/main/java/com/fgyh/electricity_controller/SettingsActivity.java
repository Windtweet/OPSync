package com.fgyh.electricity_controller;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

public class SettingsActivity extends AppCompatActivity {

    private SeekBar seekBarBatteryRange;
    private TextView tvBatteryRange;
    private Switch switchSmartPeak;
    private Switch switchLearningMode;
    private Switch switchTravelMode;
    private RelativeLayout genericAva;
    private LinearLayout btnChart;
    private LinearLayout btnSettings;
    private LinearLayout btnPic;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        seekBarBatteryRange = findViewById(R.id.seekbar_battery_range);
        tvBatteryRange = findViewById(R.id.tv_battery_range);
        switchSmartPeak = findViewById(R.id.switch_smart_peak);
        switchLearningMode = findViewById(R.id.switch_learning_mode);
        switchTravelMode = findViewById(R.id.switch_travel_mode);
        genericAva = findViewById(R.id.generic_ava);
        btnChart = findViewById(R.id.btn_chart);
        btnSettings = findViewById(R.id.btn_settings);
        btnPic = findViewById(R.id.btn_pic);

        // 设置SeekBar的监听器
        seekBarBatteryRange.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvBatteryRange.setText(progress + "%");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                // 用户开始拖动
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // 用户停止拖动
            }
        });

        // 设置Switch的监听器
        switchSmartPeak.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                String message = isChecked ? "智能调峰系统已开启" : "智能调峰系统已关闭";
                Toast.makeText(SettingsActivity.this, message, Toast.LENGTH_SHORT).show();
                // 在这里实现实际的功能
            }
        });

        switchLearningMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                String message = isChecked ? "学习模式已开启" : "学习模式已关闭";
                Toast.makeText(SettingsActivity.this, message, Toast.LENGTH_SHORT).show();
                // 在这里实现实际的功能
            }
        });

        switchTravelMode.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                String message = isChecked ? "远行模式已开启" : "远行模式已关闭";
                Toast.makeText(SettingsActivity.this, message, Toast.LENGTH_SHORT).show();
                // 在这里实现实际的功能
            }
        });

        // 设置头像点击监听器
        genericAva.setOnClickListener(v -> {
            new AlertDialog.Builder(SettingsActivity.this)
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

        // 设置底部导航按钮的点击监听器
        btnChart.setOnClickListener(v -> {
            Intent intent = new Intent(SettingsActivity.this, MainActivity.class);
            startActivity(intent);
        });

        btnSettings.setOnClickListener(v -> {
            Intent intent = new Intent(SettingsActivity.this, SettingsActivity.class);
            startActivity(intent);
        });

        btnPic.setOnClickListener(v -> {
            Intent intent = new Intent(SettingsActivity.this, ChartActivity.class);
            startActivity(intent);
        });
    }
}
