#include <ModbusRTUSlave.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <RTClib.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

#define ONE_WIRE_BUS 26

uint8_t sensor1[8] = { 0x28, 0x42, 0xA7, 0x48, 0xF6, 0x96, 0x3C, 0xB3 };
uint8_t sensor2[8] = { 0x28, 0x4E, 0x81, 0x48, 0xF6, 0x09, 0x3C, 0x71 };

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float Tem1, Tem2;

HardwareSerial MySerial(1); 
const int MySerialRX = 16;
const int MySerialTX = 17;

ModbusRTUSlave modbus(Serial1); 
const unsigned long timeout = 600;
bool dataAvailable = false;
bool coils[1];
bool discreteInputs[2];
uint16_t holdingRegisters[1];
uint16_t inputRegisters[9];

float VBUS, VBAT, IBUS, IBAT;
#define PSTOP 25
#define relay_pin1 13
#define relay_pin2 18

// 创建rtc实例
RTC_DS3231 rtc;

//OLED显示屏初始化部分
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);  //初始化U8g2设置针脚
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g22(U8G2_R0);

String State = "NG";
String PowerS = "NG";
String TollS = "NG";

int Total;

void TemDetect() {
  sensors.requestTemperatures();             // 发送命令获取温度
  Tem1 = sensors.getTempC(sensor1);
  Tem2 = sensors.getTempC(sensor2);
}

void OLED_Steup() {
  u8g2.clearBuffer();                        //清除缓存
  u8g2.setFont(u8g2_font_wqy14_t_gb2312);   //设定字体为 u8g2_font_wqy14_t_gb2312a  要想显示文字必须设置字体
  u8g2.setCursor(7, 14);                    //设定print命令的坐标位置，其中y会向下移动2个像素，可重叠使用
  u8g2.print("家庭联网储能系统");            //在点（7，14）的右上角写文字
  u8g2.setCursor(35, 30);
  u8g2.print("峰谷易和");
  u8g2.setCursor(13, 46);
  u8g2.print("物联网设计竞赛");
  u8g2.sendBuffer();                        //将刚刚在缓存中绘制的全部内容发送出去
  delay(100);                               //延时0.1秒
}

void OLED_Norm() {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.print("电量:");
  u8g2.setCursor(30, 14);
  u8g2.print(CheckBattery());
  u8g2.setCursor(55, 14);
  u8g2.print("%");
  u8g2.setCursor(80, 14);
  u8g2.print("正常");
  u8g2.setCursor(0, 27);
  u8g2.print("S1:");
  u8g2.setCursor(20, 27);
  u8g2.print(inputRegisters[2]/1000.0);
  u8g2.setCursor(45, 27);
  u8g2.print("V");
  u8g2.setCursor(70, 27);
  u8g2.print("S2:");
  u8g2.setCursor(95, 27);
  u8g2.print(inputRegisters[3]/1000.0);
  u8g2.setCursor(120, 27);
  u8g2.print("V");
  u8g2.setCursor(0, 40);
  u8g2.print("S3:");
  u8g2.setCursor(20, 40);
  u8g2.print(inputRegisters[4]/1000.0);
  u8g2.setCursor(45, 40);
  u8g2.print("V");
  u8g2.setCursor(70, 40);
  u8g2.print("S4:");
  u8g2.setCursor(95, 40);
  u8g2.print(inputRegisters[5]/1000.0);
  u8g2.setCursor(120, 40);
  u8g2.print("V");
  u8g2.setCursor(0, 51);
  u8g2.print("VBat:");
  u8g2.setCursor(32, 51);
  u8g2.print(inputRegisters[1]/1000.0);
  u8g2.setCursor(58, 51);
  u8g2.print("V");
  u8g2.setCursor(65, 51);
  u8g2.print("IBat:");
  u8g2.setCursor(95, 51);
  u8g2.print(inputRegisters[8]/1000.0);
  u8g2.setCursor(120, 51);
  u8g2.print("V");
  u8g2.setCursor(2, 63);
  u8g2.print("温度");
  u8g2.setCursor(35, 62);
  u8g2.print(Tem1);
  u8g2.setCursor(80, 62);
  u8g2.print(Tem2);
  u8g2.setCursor(115, 62);
  u8g2.print("°C");
  u8g2.sendBuffer();                 //将刚刚在缓存中绘制的全部内容发送出去
}

void OLED2_Norm() {
  u8g22.clearBuffer(); 
  u8g22.setCursor(2, 32); 
  u8g22.print("状态:");
  u8g22.setCursor(35, 32);
  u8g22.print(State);
  u8g2.setCursor(0, 62);
  u8g2.print("当前处于:");
  u8g2.setCursor(64, 62);
  u8g2.print(PowerS);
  u8g22.sendBuffer();

  DateTime now = rtc.now();
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());
  // 显示时间在OLED屏幕上
  u8g22.drawStr(0, 14, buffer);        // 将时间字符串绘制在屏幕上
  u8g22.sendBuffer();                 //将刚刚在缓存中绘制的全部内容发送出去
}

void wifi_time_setup(){
  const char *ssid = "SmartHome";    //WiFi网络名称
  const char *password = "Qc890320"; //WiFi网络密码
  const char *ntpServer = "ntp.aliyun.com";
  const long gmtOffset_sec = 8 * 3600;
  const int daylightOffset_sec = 0;

  u8g2.setCursor(5, 62);
  u8g2.print("正在联网校准时钟");
  u8g2.sendBuffer();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected!");
    // 从网络时间服务器上获取并设置时间
    // 获取成功后芯片会使用RTC时钟保持时间的更新
    u8g2.clearBuffer();
    u8g2.setCursor(15, 14);
    u8g2.print("WiFi连接成功");
    u8g2.sendBuffer();
    delay(500);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);
    struct tm timeinfo;

  if (!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    u8g2.clearBuffer();
    u8g2.setCursor(3, 14);
    u8g2.print("未能连接ntp服务器");
    u8g2.setCursor(5, 30);
    u8g2.print("使用机内RTC记忆");
    u8g2.sendBuffer();
    delay(1500);
    return;
  }

  u8g2.setCursor(0, 30);
  u8g2.print("成功连接ntp服务器");
  u8g2.sendBuffer();

  Serial.println(&timeinfo, "%F %T %A"); // 格式化输出

  int to_second = timeinfo.tm_sec;
  int to_minute = timeinfo.tm_min;
  int to_hours = timeinfo.tm_hour;
  int to_day = timeinfo.tm_mday;
  int to_month = timeinfo.tm_mon + 1;  // tm_mon 从0开始表示一月，所以需要加1
  int to_year = timeinfo.tm_year + 1900; // tm_year 是从1900开始的年份，所以需要加1900
  // 重新设定DS3231模块的时间
  rtc.adjust(DateTime(to_year, to_month, to_day, to_hours, to_minute, to_second));

  u8g2.setCursor(5, 46);
  u8g2.print("已校准好RTC时间");
  u8g2.sendBuffer();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi disconnected!");

  u8g2.setCursor(20, 62);
  u8g2.print("WiFi已断开");
  u8g2.sendBuffer();
  delay(1500);
}

uint8_t registerValues[27];
double vbat, vbus, ibus, ibat;

int Requestall() {
  Wire.beginTransmission(0x74);
  Wire.write(0x00);// 想要读取的寄存器地址
  Wire.endTransmission();
 
  Wire.requestFrom(116, 27);  // 请求27个字节的数据

  unsigned long startTime = millis();
  bool dataAvailable = false;
  while ((millis() - startTime) < timeout) {
    if (Wire.available() == 27) {
      dataAvailable = true;
      break;
    }
  }
  if (dataAvailable) {
    for (uint8_t reg = 0x00; reg <= 0x1B; reg++) {
      registerValues[reg] = Wire.read();  // 读取寄存器值
    }
    return 1;
  } else {
    return 404;
  }
}

unsigned int write10(uint16_t channel, uint8_t high_8_bits, uint8_t low_2_bits) {
  digitalWrite(PSTOP, HIGH);
  Wire.beginTransmission(0x74);
  Wire.write(channel);
  Wire.write(high_8_bits);
  Wire.write(low_2_bits);
  Wire.endTransmission();
  digitalWrite(PSTOP, LOW);

  bool dataAvailable = false;
  Wire.beginTransmission(0x74);
  Wire.write(channel);
  Wire.endTransmission();
  Wire.requestFrom(116, 2);
  unsigned long startTime = millis();
  dataAvailable = false;
  while ((millis() - startTime) < timeout) {
    if (Wire.available()) {
      dataAvailable = true;
      break;
    }
  }
  if (dataAvailable) {
    uint8_t high_8_bits2, low_8_bits2; 
    while (Wire.available()) {
      if (Wire.available() == 2){
        high_8_bits2 = Wire.read();         // 读取高8位
      }else if (Wire.available() == 1){
        low_8_bits2 = Wire.read();
      }
    }
    if (high_8_bits2 == high_8_bits && low_8_bits2 == low_2_bits){
      return 1;
    }else{
      return 0;
    }
  } else {
    return 0;
  }
}

char write8(uint16_t channel, uint8_t data) {
  digitalWrite(PSTOP, HIGH);
  Wire.beginTransmission(0x74);
  Wire.write(channel);
  Wire.write(data);
  Wire.endTransmission();
  digitalWrite(PSTOP, LOW);

  bool dataAvailable = false;
  Wire.beginTransmission(0x74);
  Wire.write(channel);
  Wire.endTransmission();
  Wire.requestFrom(116, 1);
  unsigned long startTime = millis();
  dataAvailable = false;
  while ((millis() - startTime) < timeout) {
    if (Wire.available()) {
      dataAvailable = true;
      break;
    }
  }
  if (dataAvailable) {
    uint8_t value;
    while (Wire.available()) {
      value = Wire.read();         // 读取8位
    }
    if (value == data){
      return 1;
    }else{
      return 0;
    }
  } else {
    return 0;
  }
}

unsigned int Get10(unsigned int channel){    //读10位
  uint8_t high_8_bits, low_8_bits;
  uint16_t value;
  high_8_bits = registerValues[channel];         // 读取高8位
  low_8_bits = registerValues[channel+1];
  value = ((uint16_t)high_8_bits << 2) | ((low_8_bits >> 6) & 0x03);
  return value;
}

unsigned int Get8(unsigned int channel) {
  uint8_t value;
  value = registerValues[channel];         // 读取8位
  return value;
}

char adc_control(String switcher){
  uint8_t k;
  k = Get8(0x0c);
    if (switcher == "on"){
      k |= 0x20;
    }else if (switcher == "off"){
      k &= 0xdf;
    }
    char j = write8(0x0c, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

char out_control(String switcher){
  uint8_t k;
  k = Get8(0x09);
    if (switcher == "on"){
      k |= 0x80;
    }else if (switcher == "off"){
      k &= 0x7f;
    }
    char j = write8(0x09, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

char dead_control(int dtime){
  uint8_t k;
  k = Get8(0x09);
    if (dtime == 80){
      k |= 0x03;
    }else if (dtime == 60){
      k |= 0x02;
      k &= 0xfe;
    }else if (dtime == 40){
      k |= 0x01;
      k &= 0xfd;
    }else if (dtime == 20){
      k &= 0xfc;
    }
    char j = write8(0x09, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

char VBUSExt(String switcher){
  uint8_t k;
  k = Get8(0x0a);
    if (switcher == "on"){
      k |= 0x10;
    }else if (switcher == "off"){
      k &= 0xef;
    }
    char j = write8(0x0a, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

char VbatExtSet(){
  uint8_t k;
  //k = Get8(0x00);
    k = 0x20;
    char j = write8(0x00, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

char Vout_set(double Vset){
  unsigned int V = (((Vset / (1+(100.0/13)))*1000)/2) - 1;
  uint8_t high_8_bits;
  uint8_t low_2_bits;
  // 提取高8位
  high_8_bits = (V >> 2) & 0xFF;
  // 提取低2位并将它们移到8位变量的高2位
  low_2_bits = (V & 0x03) << 6;

  char j = write10(0x03, high_8_bits, low_2_bits);
  if (j == 1){
    return 1;
  }else{
    return 0;
  }
}

char IBatLimitSet(double IBatSet){
  uint8_t I = ((IBatSet *256) / 12) - 1;
  char j = write8(0x06, I);
  if (j == 1){
    return 1;
  }else{
    return 0;
  }
}

char IBusLimitSet(double IBatSet){
  uint8_t I = ((IBatSet *256) / 6) - 1;
  char j = write8(0x05, I);
  if (j == 1){
    return 1;
  }else{
    return 0;
  }
}

char CurentRatio(String switcher){
  uint8_t k;
  k = Get8(0x08);
    if (switcher == "6x"){
      k |= 0x04;
      k &= 0xf7;
    }else if (switcher == "12x"){
      k |= 0x08;
      k &= 0xfb;
    }
    char j = write8(0x08, k);
    if (j == 1){
      return 1;
    }else{
      return 0;
    }
}

void GetADC(){
  vbat = ((Get10(0x0F)+1)*12.5*2)/1000;  // VBAT_FB_VALUE
  vbus = ((Get10(0x0D)+1)*12.5*2)/1000 ;  // VBUS_FB_VALUE   = (4 x VBUS_FB_VALUE + VBUS_FB_VALUE2 + 1) x VBUS_RATIO x 2 mV
  ibus = ((Get10(0x11)+1)*2.0*6.0)/1200.0;  // IBUS_VALUE
  ibat = ((Get10(0x13)+1)*2.0*12.0)/1200.0;  // IBAT_VALUE
}

void SerialPrintall(){
  for (uint8_t reg = 0x00; reg < 27; reg++) {
    Serial.print("0x");
    Serial.print(reg, HEX);
    Serial.print(": ");
    Serial.println(registerValues[reg], BIN);
  }
}

void SerialPrintVI(){
  Serial.print("VBAT: ");
  Serial.print(vbat);
  Serial.print(", VBUS: ");
  Serial.print(vbus);
  Serial.print(", IBUS: ");
  Serial.print(ibus);
  Serial.print(", IBAT: ");
  Serial.println(ibat);
}

unsigned int CheckBattery() {
  const int numReadings = 50;    // 读取的次数
  long totalValue = 0;           // 用于累加读取值
  int analogPin = 34;            // 模拟输入引脚

  for (int i = 0; i < numReadings; i++) {
    totalValue += analogRead(analogPin);
  }

  // 计算平均值
  float averageValue = totalValue / (float)numReadings;
  inputRegisters[1] = (averageValue/1.5)*(1.5+5.1) -2000;
  // 将平均值转换为电池电量百分比
  unsigned int batteryLevel = ((averageValue - 3383.0) / 712.0) * 100-20;

  return batteryLevel;
}

void GetCell() {
  inputRegisters[2] = 3306;
  inputRegisters[3] = 3308;
  inputRegisters[4] = 3305;
  inputRegisters[5] = 3302;
}

void printmodbus(){
  // 打印discreteInputs数组的值
  for (int i = 0; i < 2; i++) {
    Serial.print("discreteInputs[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(discreteInputs[i]);
  }

  // 打印coils数组的值
  for (int i = 0; i < 1; i++) {
    Serial.print("coils[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(coils[i]);
  }

  // 打印holdingRegisters数组的值
  for (int i = 0; i < 1; i++) {
    Serial.print("holdingRegisters[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(holdingRegisters[i]);
  }

  // 打印inputRegisters数组的值
  for (int i = 0; i < 9; i++) {
    Serial.print("inputRegisters[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(inputRegisters[i]);
  }
}

void setup() {
  sensors.begin();

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2_SetI2CAddress(u8g22.getU8g2(), 0x3d*2);
  u8g22.begin();
  u8g22.enableUTF8Print();
  u8g22.setFont(u8g2_font_wqy14_t_gb2312);
  Wire.begin();
  Serial.begin(115200);
  Wire.setClock(100000);  //可接受的值为 100000（标准模式）和 400000（快速模式）。某些处理器还支持 10000（低速模式）、1000000（快速模式增强）和 3400000（高速模式）。

  MySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
  modbus.configureCoils(coils, 1);
  modbus.configureDiscreteInputs(discreteInputs, 2);
  modbus.configureHoldingRegisters(holdingRegisters, 1);
  modbus.configureInputRegisters(inputRegisters, 9);
  modbus.begin(100, 115200);

  discreteInputs[0] = 0;
  discreteInputs[1] = 0;
  coils[0] = 1;
  holdingRegisters[0] = 0;
  inputRegisters[0] = 0;
  inputRegisters[1] = 0;
  inputRegisters[2] = 0;
  inputRegisters[3] = 0;
  inputRegisters[4] = 0;
  inputRegisters[5] = 0;
  inputRegisters[6] = 0;
  inputRegisters[7] = 0;
  inputRegisters[8] = 0;

  OLED_Steup();

  // 初始化rtc，
  if (! rtc.begin()) { // 若果初始化失败
    Serial.println("Couldn't find RTC");
    Serial.flush(); 
    abort();  // 程序停止运行
  }
  wifi_time_setup();   //联网获取时间

  pinMode(PSTOP, OUTPUT);
  digitalWrite(PSTOP, LOW);

  pinMode (relay_pin1, OUTPUT);
  digitalWrite(relay_pin1, LOW);

  pinMode (relay_pin2, OUTPUT);
  digitalWrite(relay_pin2, LOW);

  const unsigned long timeout2 = 20000;
  unsigned long startTime = millis();
  bool dataAvailable = false;
  int result;
  while ((millis() - startTime) < timeout2) {
    result = Requestall();
    if (result == 1) {
      dataAvailable = true;
      break;
    }
  }
  if (dataAvailable) {
    adc_control("on");
    Vout_set(12.00);
    dead_control(20);
    VbatExtSet();
    VBUSExt("on");
    CurentRatio("6x");
    IBatLimitSet(4.00);
    IBusLimitSet(6.00);
    out_control("on");
  } else {
    Serial.println("Couldn't find SC8815");
    Serial.flush(); 
    abort();  // 程序终止运行
  }
}

void loop() {
  int result = Requestall();
  if (result == 404){
  }
  else if (result == 1){
    //SerialPrintall();
    GetADC();
    //SerialPrintVI();
    //delay(100);
    inputRegisters[6] = vbus*ibus;
    inputRegisters[7] = vbus*1000;
    inputRegisters[8] = ibus*1000;
  }
  TemDetect();
  OLED_Norm();
  OLED2_Norm();
  GetCell();
  DateTime now = rtc.now();
  int hour = now.hour();    // 获取当前小时

  if ((hour >= 7 && hour < 11) || (hour >= 13 && hour < 22)) {   //峰电期间
    PowerS = "峰电期间";
    int Total2 = CheckBattery();
    inputRegisters[0] = Total2;
    if (Total2 >= 20){
      State = "放电中";
      discreteInputs[1] = 1;
    }else{
      State = "停止放电";
      discreteInputs[1] = 0;
    }
  } else {                //谷电期间
    PowerS = "谷电期间";
    int Total2 = CheckBattery();
    inputRegisters[0] = Total2;
    if (Total2 >= 90){
      State = "停止充电";
      digitalWrite(relay_pin1, LOW);
      discreteInputs[0] = 0;
    }else if(Total2 >= 100){
      State = "停止充电";
      digitalWrite(relay_pin1, LOW);
      discreteInputs[0] = 0;
    }else{
      State = "充电中";
      digitalWrite(relay_pin1, HIGH);
      discreteInputs[0] = 1;
    }
  }
  printmodbus();
  modbus.poll();
}
