/*
   1.Serial I/O 測試
   2.把SerialUSB得到的字串，用OLED顯示出來
   3.睡眠模式測試：睡眠1.8mA 運轉18mA (不包含OLED)
*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping
//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 17;
const byte month = 11;
const byte year = 15;

String serialGet(Stream &_serial)
{
  String _str;
  while (_serial.available())
  {
    char _c = _serial.read();
    _str += _c;
  }
  return _str;
}

void serialSend(Stream&_serial, String _str, bool _EOL)
{
  if (_EOL)  _serial.println(_str);
  else      _serial.print(_str);
}


void OLED(String _content)
{
  u8g2.setFont(u8g2_font_cu12_tr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 15);
    u8g2.print(_content.c_str());
  } while ( u8g2.nextPage() );
}


void showMsg(String _msg, int _state = 0)
{
  switch (_state) {
    case 0:
      SerialUSB.println(_msg);
      OLED(_msg);
      break;
    case 1:
      OLED(_msg);
      break;
    case 2:
      SerialUSB.println(_msg);
      break;
  }
  delay(1000);
}


void setup()
{
  u8g2.begin();

  /*
     SerialUSB 是接USB線開啟監控埠後Debug用
     若沒有開啟監控埠，則會卡在while(!SerialUSB)的當中
  */
  SerialUSB.begin(9600);

  int _whileCount = 0;
  while (!SerialUSB) {
    if (_whileCount < 1000) {
      _whileCount++;
      delay(1);
    }
    else break;
  }
  showMsg("System Ready");

  rtc.begin();

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);
  showMsg("RTC Ready");

  rtc.setAlarmTime(17, 00, 10);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  showMsg("Alarm Ready: 10s");

  rtc.attachInterrupt(alarmMatch);
  showMsg("SLEEP");
  rtc.standbyMode();
}

int count = 0;

void loop()
{
  
  showMsg("loop:"+String(count), 1);
  count++;
  
  String _serialStr;
  _serialStr = serialGet(SerialUSB);
  if (_serialStr.length() > 0) {
    serialSend(SerialUSB, "Inupt: " + _serialStr, true);
    OLED(_serialStr);
  }
  delay(1000);
}

void alarmMatch()
{
  USBDevice.detach();
  digitalWrite(5, HIGH);
  delay(500);
  for (int _i = 0; _i < 5; _i++)
  {
    digitalWrite(5, LOW);
    delay(500);
    digitalWrite(5, HIGH);
    delay(500);
  }
  USBDevice.attach();
  showMsg("WAKE UP!!", 1);
  delay(2000);
  showMsg("Reset Serial port!!", 1);
  delay(2000);
}
