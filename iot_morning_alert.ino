#define M5STACK_MPU6886 

#include <M5Stack.h>
#include <WiFi.h>
#include "time.h"

#include <HTTPClient.h>
const char *server = "hooks.slack.com";
const char *json_text = "{\"text\":\"Good Morning!\",\"username\":\"Sora\"}";

const char * slack_sub_ca = 
"-----BEGIN CERTIFICATE-----\n"

"-----END CERTIFICATE-----\n" ;

HTTPClient http;

const char* ssid     = "ssid";
const char* password = "password";

const char* ntpServer = "ntp.nict.jp";
const long  gmtOffset_sec = 3600 * 9;
const int   daylightOffset_sec = 0;

int selector = 0;

struct tm timeinfo_now;

struct tm timeinfo;
int month;
int day;
int hour;
int minute;

void printTime(int hour, int minute) {
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(40,80);
  M5.Lcd.printf("%02d:%02d", hour, minute);
}

void printDate(int month, int day, int hour, int minute) {
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(40,120);
  M5.Lcd.printf("%02d-%02d %02d:%02d", month, day, hour, minute);
}

void send_to_slack() {
  M5.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  http.begin(server, 443, "/services/xxx/yyy/zzz", slack_sub_ca);
  http.addHeader("Content-Type", "application/json");
  http.POST((uint8_t*)json_text, strlen(json_text));
}

void setup() {
  M5.begin();
  M5.Power.begin();
  
  M5.Lcd.print("Connecting to YOUR_SSID ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
  }
  M5.Lcd.println(" CONNECTED");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime(&timeinfo);
  month  = timeinfo.tm_mon+1;
  day    = timeinfo.tm_mday;
  hour   = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  printTime(hour, minute);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  M5.update();
  if (selector==0) {
    if (M5.BtnC.wasReleased()) {
      minute--;
      if (minute < 0) {
        minute = 59;
      }
    } else if (M5.BtnB.wasReleased()) {
      minute++;
      if (minute > 59) {
        minute = 0;
      }
    } else if (M5.BtnA.wasReleased()) {
      selector++;
    }
    printTime(hour, minute);
    delay(10);
  } else if (selector==1) {
    if (M5.BtnC.wasReleased()) {
      hour--;
      if (hour < 0) {
        hour = 23;
      }
    } else if (M5.BtnB.wasReleased()) {
      hour++;
      if (hour > 23) {
        hour = 0;
      }
    } else if (M5.BtnA.wasReleased()) {
      selector++;
    }
    printTime(hour, minute);
    delay(10);
  } else {
    getLocalTime(&timeinfo_now);
    printDate(month, day, hour, minute);
    printDate(timeinfo_now.tm_mon+1, timeinfo_now.tm_mday, timeinfo_now.tm_hour, timeinfo_now.tm_min);
    
    if (timeinfo_now.tm_hour==hour & timeinfo_now.tm_min==minute) {
      M5.IMU.Init();
        
      M5.Speaker.begin();
      M5.Speaker.beep();

      int s_count = 0;
      int th = 2;

      float accX = 0.0F;
      float accY = 0.0F;
      float accZ = 0.0F;
      
      while (1==1) {
        
        M5.IMU.getAccelData(&accX,&accY,&accZ);

        if (accX > th | accX < -th) {
          s_count++;
        } else if (accY > th | accY < -th) {
          s_count++;
        } else if (accZ > th | accZ < -th) {
          s_count++;
        }
        
        if (s_count > 1000) {
          M5.Speaker.mute();
          selector = 0;
          send_to_slack();
          M5.Lcd.clear(BLACK);
          break;
        }
        delay(1);
      }
    }
    delay(1000);
  }
}
