package com.fgyh.electricity_controller;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {

    private RelativeLayout ivProfile;
    private LinearLayout btnChart;
    private LinearLayout btnSettings;
    private LinearLayout btnPic;
    private TextView kw_h_7_12_2;
    private Handler mainHandler;
    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ivProfile = findViewById(R.id.generic_ava);
        btnChart = findViewById(R.id.btn_chart);
        btnSettings = findViewById(R.id.btn_settings);
        btnPic = findViewById(R.id.btn_pic);
        kw_h_7_12_2 = findViewById(R.id.kw_h_7_12_2);
        mainHandler = new Handler(Looper.getMainLooper());

        ivProfile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new AlertDialog.Builder(MainActivity.this)
                        .setTitle("退出登陆吗？")
                        .setMessage("如果退出，你将需要重新登陆。")
                        .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Intent intent = new Intent(MainActivity.this, LoginActivity.class);
                                startActivity(intent);
                                finish();
                            }
                        })
                        .setNegativeButton("取消", null)
                        .show();
            }
        });

        btnChart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, MainActivity.class);
                startActivity(intent);
            }
        });

        btnSettings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, SettingsActivity.class);
                startActivity(intent);
            }
        });

        btnPic.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, ChartActivity.class);
                startActivity(intent);
            }
        });

        // 从服务器获取数据
        new FetchDataTask().execute();
    }

    private class FetchDataTask {
        private final String urlString = "https://api.coze.cn/v3/chat";
        private final String personalAccessToken = "pat_nDY12f91pWCgdwxXnJMkGaqaEmRn6Gjpe7jqMNr38rSVDEThVj89vu2ZTnUS0Lep";
        private final String botId = "7391388797302620201";
        private final String userId = "2241256603257884";
        private final String yourQuery = "请帮我根据数据库中的内容，告诉我一段话，描述该月峰谷电用电情况，字数和格式上与以下这段话类似，字数禁止超过200字：该用户本月用电习惯良好，在峰谷易和的调节下，本月总计为用户节约了71kW·h的峰电，用户用电最多的是7月12日，用掉了21kW·h的电，最少的是7月20日，用掉了7kW·h的电。希望用户再接再厉，继续保持!";

        private final ExecutorService executorService = Executors.newSingleThreadExecutor();

        public void execute() {
            executorService.execute(this::fetchDataFromServer);
        }

        private void fetchDataFromServer() {
            HttpURLConnection urlConnection = null;
            BufferedOutputStream out = null;
            try {
                URL url = new URL(urlString);
                urlConnection = (HttpURLConnection) url.openConnection();
                urlConnection.setRequestMethod("POST");
                urlConnection.setRequestProperty("Authorization", "Bearer " + personalAccessToken);
                urlConnection.setRequestProperty("Content-Type", "application/json");
                urlConnection.setDoOutput(true);

                // 构建请求体
                String jsonInputString = "{"
                        + "\"bot_id\": \"" + botId + "\","
                        + "\"user_id\": \"" + userId + "\","
                        + "\"stream\": false,"
                        + "\"auto_save_history\": true,"
                        + "\"additional_messages\": ["
                        + "    {"
                        + "        \"role\": \"user\","
                        + "        \"content\": \"" + yourQuery + "\","
                        + "        \"content_type\": \"text\""
                        + "    }"
                        + "]"
                        + "}";

                // 发送请求体
                out = new BufferedOutputStream(urlConnection.getOutputStream());
                byte[] input = jsonInputString.getBytes(StandardCharsets.UTF_8);
                out.write(input, 0, input.length);
                out.flush();
                Thread.sleep(10000); // 等待10秒再检查
                // 读取响应
                BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
                StringBuilder stringBuilder = new StringBuilder();
                String line;
                while ((line = bufferedReader.readLine()) != null) {
                    stringBuilder.append(line).append("\n");
                }
                bufferedReader.close();
                String response = stringBuilder.toString();

                // 输出完整响应内容以便检查
                //Log.d(TAG, "API Response: " + response);

                // 处理响应
                JSONObject jsonObject = new JSONObject(response);
                JSONObject data = jsonObject.getJSONObject("data");
                String status = data.getString("status");

                // 等待请求完成
                while ("in_progress".equals(status)) {
                    status = checkRequestStatus(data.getString("conversation_id"));
                }

                if ("completed".equals(status)) {
                    JSONArray messages = data.getJSONArray("messages");
                    StringBuilder messageBuilder = new StringBuilder();
                    for (int i = 0; i < messages.length(); i++) {
                        JSONObject messageObj = messages.getJSONObject(i);
                        messageBuilder.append(messageObj.getString("content")).append("\n");
                    }
                    String finalMessage = messageBuilder.toString();

                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            kw_h_7_12_2.setText(finalMessage);
                        }
                    });
                } else {
                    mainHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            kw_h_7_12_2.setText("加载数据失败。");
                        }
                    });
                }
            } catch (Exception e) {
                Log.e(TAG, "获取数据时出错", e);
                mainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        kw_h_7_12_2.setText("加载数据失败。");
                    }
                });
            } finally {
                if (out != null) {
                    try {
                        out.close();
                    } catch (Exception e) {
                        Log.e(TAG, "关闭输出流时出错", e);
                    }
                }
                if (urlConnection != null) {
                    urlConnection.disconnect();
                }
            }
        }

        private String checkRequestStatus(String conversationId) {
            HttpURLConnection urlConnection = null;
            try {
                String urlString = "https://api.coze.cn/v3/chat/status";
                URL url = new URL(urlString);
                urlConnection = (HttpURLConnection) url.openConnection();
                urlConnection.setRequestMethod("POST");
                urlConnection.setRequestProperty("Authorization", "Bearer " + personalAccessToken);
                urlConnection.setRequestProperty("Content-Type", "application/json");
                urlConnection.setDoOutput(true);

                // 构建请求体
                String jsonInputString = "{"
                        + "\"bot_id\": \"" + botId + "\","
                        + "\"conversation_id\": \"" + conversationId + "\""
                        + "}";

                // 发送请求体
                try (BufferedOutputStream out = new BufferedOutputStream(urlConnection.getOutputStream())) {
                    byte[] input = jsonInputString.getBytes(StandardCharsets.UTF_8);
                    out.write(input, 0, input.length);
                }

                // 读取响应
                BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
                StringBuilder stringBuilder = new StringBuilder();
                String line;
                while ((line = bufferedReader.readLine()) != null) {
                    stringBuilder.append(line).append("\n");
                }
                bufferedReader.close();
                String response = stringBuilder.toString();

                // 处理响应
                JSONObject jsonObject = new JSONObject(response);
                JSONObject data = jsonObject.getJSONObject("data");
                return data.getString("status");
            } catch (Exception e) {
                Log.e(TAG, "检查请求状态时出错", e);
                return "error";
            } finally {
                if (urlConnection != null) {
                    urlConnection.disconnect();
                }
            }
        }
    }
}
