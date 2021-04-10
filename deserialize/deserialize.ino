#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "SH1106Wire.h"
#include "time.h"
#include <ArduinoJson.h>

const char* ssid = "your_ssid";
const char* pass = "your_pass";
String city = "your_city";
String api_key = "your_key";

SH1106Wire display(0x3c, 21, 22);
long last_epoch = 0;

// 現在時刻の取得 get current time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup() {
  // シリアルポートの初期化 initialize serial port
  Serial.begin(115200);

  // OLEDの初期化 initialize oled
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Initializing...");
  display.display();

  // アクセスポイントに接続 connect to access point
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.clear();
  display.drawString(0, 0, "WiFi Connected.");
  display.display();

  // 時計の初期化 initialize clock
  configTime(3600, 3600, "ntp.nict.jp");
  delay(1000);
}

void loop() {
  HTTPClient http;
  time_t now;

  // 1分ごとに天気等の情報を取得 get weather information every minute
  now = getTime();
  if (now - last_epoch >= 60) {
    // Open Weather Mapから天気等の情報を取得
    // Get weather information from Open Weather Map
    StaticJsonDocument<1024> doc;
    String url = "http://api.openweathermap.org/data/2.5/weather?q="
               + city + "&appid=" + api_key + "&units=metric";
    display.clear();
    if (http.begin(url)) {
      int status = http.GET();
      if (status > 0) {
        if (status == HTTP_CODE_OK) {
          // JSONを変換 deserialize JSON
          DeserializationError err = deserializeJson(doc, http.getString());
          if (err) {
            display.drawString(0, 0, err.c_str());
          }
          else {
            char buf[10];
            // 気温を表示 display temperature
            double temp = doc["main"]["temp"];
            String msg = "Temperature: ";
            dtostrf(temp, 6, 2, buf);
            msg.concat(buf);
            display.drawString(0, 0, msg);
            // 天気を表示 display weather
            const char* weather = doc["weather"][0]["main"];
            msg = "Weather: ";
            msg.concat(weather);
            display.drawString(0, 12, msg);
            // 気圧を表示 display pressure
            int pressure = doc["main"]["pressure"];
            msg = "Pressure: ";
            sprintf(buf, "%4d", pressure);
            msg.concat(buf);
            display.drawString(0, 24, msg);
            // 湿度を表示 display humidity
            int humidity = doc["main"]["humidity"];
            msg = "Humidity: ";
            sprintf(buf, "%3d", humidity);
            msg.concat(buf);
            display.drawString(0, 36, msg);
          }
        }
        else {
          Serial.print("HTTP Error ");
          Serial.println(status);
        }
      }
      else {
        Serial.println("Get Failed");
      }
      http.end(); 
    }
    else {
      Serial.print("Connect error");
    }
    display.display();
    last_epoch = now;   
  }
  else {
    Serial.println("Waiting");
  }
}
