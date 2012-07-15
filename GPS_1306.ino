// Code for Adafruit GPS modules using MTK3329/MTK3339 driver
//
// This code shows how to listen to the GPS module in an interrupt
// which allows the program to have more 'freedom' - just parse
// when a new NMEA sentence is available! Then access data when
// desired.
//
// Tested and works great with the Adafruit Ultimate GPS module
// using MTK33x9 chipset
//    ------> http://www.adafruit.com/products/746
// Pick one up today at the Adafruit electronics shop 
// and help support open source hardware & software! -ada
//
// Also pick up an OLED Display at the Adafruit sore!
//    ------> http://www.adafruit.com/products/661
// Sketch by Jeremy Saglimbeni - 07/14/12
//    ------> http://thecustomgeek.com/2012/07/14/really-smalls-gps/
#define OLED_DC 4
#define OLED_CS 2
#define OLED_CLK 5
#define OLED_MOSI 6
#define OLED_RESET 3
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9);
#define GPSECHO  true
Adafruit_GPS GPS(&mySerial);
boolean usingInterrupt = false;
int mode = 0;
int left = A5;
int mid = A3;
int right = A4;
int power = 0;
int tzhour;
int fixflag = 0;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  pinMode(left, INPUT);
  pinMode(mid, INPUT);
  pinMode(right, INPUT);
  digitalWrite(left, HIGH);
  digitalWrite(mid, HIGH);
  digitalWrite(right, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("  Where am I anyway?");
  display.println(" ");
  display.println("  Jeremy Saglimbeni");
  display.print("  thecustomgeek.com");
  display.display();
  delay(1000);
  display.clearDisplay();
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  useInterrupt(true);
  delay(1000);
}

SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  if (GPSECHO)
    if (c) UDR0 = c;  
}

void useInterrupt(boolean v) {
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else {
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();

void loop() {
  if (digitalRead(mid) == LOW) {
    if (mode == 5) {
      power++;
      if (power == 2) {
        power = 0;
      }
      if (power == 0) {
        digitalWrite(A0, HIGH);
        digitalWrite(7, HIGH);
        digitalWrite(13, HIGH);
        display.begin(SSD1306_SWITCHCAPVCC);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 8);
        display.setTextColor(WHITE);
        display.print("I'm Awake!");
        display.display();
        delay(1500);
        digitalWrite(13, LOW);
        display.clearDisplay();
        mode = 0;
      }
      if (power == 1) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 8);
        display.setTextColor(WHITE);
        display.print("Goodnight!");
        display.display();
        delay(1500);
        digitalWrite(A0, LOW);
        digitalWrite(7, LOW);
      }
      delay(250);
    }
  }

  if (digitalRead(left) == LOW) {
    if (mode == 5) {
      return;
    }
    mode++;
    if (mode == 5) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,8);
      display.print("Sleep Mode");
      display.display();
      delay(1500);
      return;
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,8);
    display.print("  Mode ");
    display.print(mode);
    display.display();
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    display.clearDisplay();
  }
  if (digitalRead(right) == LOW) {
    if (power == 1) {
      return;
    }
    if (mode == 0) {
      return;
    }
    mode--;
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,8);
    display.print("  Mode ");
    display.print(mode);
    display.display();
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    display.clearDisplay();
  }

  if (! usingInterrupt) {
    char c = GPS.read();
    if (GPSECHO)
      if (c) UDR0 = c;
  }

  if (GPS.newNMEAreceived()) {

    if (!GPS.parse(GPS.lastNMEA()))
      return;
  }

  if (timer > millis())  timer = millis();

  if (millis() - timer > 1000) {
    timer = millis(); // reset the timer

    if (mode == 0) {
      display.fillRect(0, 0, 127, 8, BLACK);
      display.setCursor(0, 0);
      display.setTextSize(1);
      if (GPS.hour >= 0 && GPS.hour < 4) {
        tzhour = GPS.hour + 20;
      }
      else {
        tzhour = GPS.hour - 4;
      }
      if (tzhour < 10) {
        display.print("0");
      }
      display.print(tzhour, DEC); 
      display.print(':');
      if (GPS.minute < 10) {
        display.print("0");
      }
      display.print(GPS.minute, DEC); 
      display.print(':');
      if (GPS.seconds < 10) {
        display.print("0");
      }
      display.print(GPS.seconds, DEC);    

      display.print("   ");
      if (GPS.month < 10) {
        display.print("0");
      }
      display.print(GPS.month, DEC);
      display.print("/");
      if (GPS.day < 10) {
        display.print("0");
      }
      display.print(GPS.day, DEC);
      display.print("/");
      display.print("20");
      display.print(GPS.year, DEC);
      display.display();

      if (GPS.fix == 0) {
        display.setTextSize(1);
        display.setCursor(0, 16);
        display.print("Acquiring Satellites!");
      }
      if (GPS.fix) {
        fixflag = 1;
        display.fillRect(0, 8, 128, 24, BLACK);
        display.setCursor(0, 8);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 16);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 24);
        display.print("Alt:");
        display.print(GPS.altitude);
        display.setCursor(92, 24);
        if (GPS.fix == 1) {
          display.print("Fix:");
          display.print(GPS.satellites);
        }
        if (GPS.fix == 0) {
          display.setTextColor(BLACK, WHITE);
          display.print("NO FIX");
          display.setTextColor(WHITE);
        }
      }
      display.display();
    }

    if (mode == 1) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(1);
      if (GPS.fix) {
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 8);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 16);
        display.print("Speed(MPH):");
        display.print(GPS.speed * 1.15); //*converts to MPH
        display.setCursor(0, 24);
        display.print("Bearing:");
        display.print(GPS.angle);
      }
      display.display();
    }

  }
  if (mode == 3) {
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    if (GPS.hour >= 0 && GPS.hour < 4) {
      tzhour = GPS.hour + 20;
    }
    else {
      tzhour = GPS.hour - 4;
    }
    if (tzhour < 10) {
      display.print("0");
    }
    display.print(tzhour, DEC); 
    display.print(':');
    if (GPS.minute < 10) {
      display.print("0");
    }
    display.print(GPS.minute, DEC);
    display.display();
  }
  if (mode == 2) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    if (GPS.hour >= 0 && GPS.hour < 4) {
      tzhour = GPS.hour + 20;
    }
    else {
      tzhour = GPS.hour - 4;
    }
    if (tzhour < 10) {
      display.print("0");
    }
    display.print(tzhour, DEC); 
    display.print(':');
    if (GPS.minute < 10) {
      display.print("0");
    }
    display.print(GPS.minute, DEC);
    display.print(":");
    if (GPS.seconds < 10) {
      display.print("0");
    }
    display.print(GPS.seconds, DEC);
    display.setCursor(0, 16);
    if (GPS.month < 10) {
      display.print("0");
    }
    display.print(GPS.month, DEC);
    display.print("/");
    if (GPS.day < 10) {
      display.print("0");
    }
    display.print(GPS.day, DEC);
    display.print("/");
    display.print("20");
    display.print(GPS.year, DEC);
    display.display();
  }

  if (mode == 4) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(GPS.speed * 1.15);
    display.print(" MPH");
    display.setCursor(0, 16);
    display.print(GPS.angle);
    display.print(" DEG");
    display.display();
  }
  if (mode == 5) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("Power Down");
    display.setCursor(0, 16);
    display.print(" Really?");
    display.display();
  }
}
