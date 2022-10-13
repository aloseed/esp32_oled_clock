/*
   ESP32 OLED Clock

   NTPサーバーで時刻合わせをしたのち、OLED(SSD1306)で時刻を表示します。

   Licence:
      [MIT](https://github.com/aloseed/esp32_oled_clock/LICENSE)

   Author:
      [alocchi](https://twitter.com/aloseed_blog)

   Blog:
      [aloseed](https://aloseed.com)
*/
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>

// U8g2コンストラクタ
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// 曜日表示文字
const char *weekChar[7] = {"日", "月", "火", "水", "木", "金", "土"};

// WiFi接続情報
const char *ssid = "**********";
const char *password = "**********";

// NTP接続情報
// 日本標準時に直結した時刻サーバ https://jjy.nict.go.jp/tsp/PubNtp/index.html
const char *ntpServer1 = "ntp.nict.jp";
// PUBLIC NTP https://www.mfeed.ad.jp/ntp/detail.html
const char *ntpServer2 = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec = 3600 * 9; // JST-9 Asia/Tokyo 3600 * 9
const int daylightOffset_sec = 0;

// 時刻情報変数
tm timeInfo;

void setup(void)
{
   // Wire.begin(5, 4);    // LOLIN(OLED付きESP32)の場合
   u8g2.begin(); // OLED初期化
   // u8g2.setFlipMode(1); // 引数のデフォルトは0、1で180°回転
   u8g2.setContrast(1);
   // Serial.begin(115200);

   u8g2.clearBuffer();
   u8g2.setFont(u8g2_font_crox1hb_tf);
   u8g2.drawStr(0, 17, "WiFi connecting...");
   u8g2.sendBuffer();

   // WiFi接続
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED)
   {
      delay(500);
   }
   u8g2.clearBuffer();
   u8g2.drawStr(0, 17, "WiFi connected");
   u8g2.sendBuffer();

   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
   if (!getLocalTime(&timeInfo)) // NTP初回時刻合わせ
   {
      // 時刻取得失敗時は停止
      Serial.println("Failed to obtain time");
      for (;;)
         ;
   }

   // WiFi切断
   WiFi.disconnect(true);
   WiFi.mode(WIFI_OFF);
}

void loop()
{
   static int currentSec = -1;
   static char buf[20];
   static time_t t;

   getLocalTime(&timeInfo);
   if (currentSec == timeInfo.tm_sec)
   {
      // 時刻が更新されていない時は描画しない
      return;
   }
   currentSec = timeInfo.tm_sec;

   u8g2.clearBuffer();
   // 時計用フォントをセット
   u8g2.setFont(u8g2_font_crox5h_tn);
   // 時分秒の描画
   sprintf(buf, "%2d", timeInfo.tm_hour);
   u8g2.drawStr(0, 17, buf);
   sprintf(buf, "%02d", timeInfo.tm_min);
   u8g2.drawStr(45, 17, buf);
   sprintf(buf, "%02d", timeInfo.tm_sec);
   u8g2.drawStr(87, 17, buf);

   // 年月日の描画
   sprintf(buf, "%4d", timeInfo.tm_year + 1900);
   u8g2.drawStr(0, 40, buf);
   sprintf(buf, "%2d", timeInfo.tm_mon + 1);
   u8g2.drawStr(0, 64, buf);
   sprintf(buf, "%2d", timeInfo.tm_mday);
   u8g2.drawStr(48, 64, buf);

   // 曜日()の描画
   u8g2.setFont(u8g2_font_crox4h_tf);
   u8g2.drawStr(96, 60, "(");
   u8g2.drawStr(122, 60, ")");

   // 年月日時分秒の描画
   // 日本語フォントをセット
   u8g2.setFont(u8g2_font_b16_b_t_japanese1);
   u8g2.drawUTF8(28, 15, "時");
   u8g2.drawUTF8(71, 15, "分");
   u8g2.drawUTF8(113, 15, "秒");
   u8g2.drawUTF8(54, 38, "年");
   u8g2.drawUTF8(28, 62, "月");
   u8g2.drawUTF8(76, 62, "日");
   u8g2.drawUTF8(104, 62, weekChar[timeInfo.tm_wday]);

   u8g2.sendBuffer();
}