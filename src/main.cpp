#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ThingSpeak.h>
#include <Adafruit_MCP9808.h>
#include <Wire.h>
#include <math.h>

#include <SSD1306.h>
#include <SSD1306Ui.h>

#include "images.h"
#include "fonts.h"

// Pin definitions for I2C
#define OLED_SDA    4  // pin 14
#define OLED_SDC    5  // pin 12
#define OLED_ADDR   0x3C
SSD1306   display(OLED_ADDR, OLED_SDA, OLED_SDC);    // For I2C
SSD1306Ui ui     ( &display );


#include "settings.h"

const char* ssid = SETTINGS_SSID;
const char* password = SETTINGS_PASS;

volatile float f;
volatile float c;
volatile float h;

void setupTempSensor(void);
void readTemp(void);
void postValues(void);
void setupOled(void);
String convertFloatToString(float); 

WiFiClient client;
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

int main() {
  setup();
  for(;;) {
    loop();
  }
}


void postValues(void) {
  if (!isnan(f)) {
    Serial.println("posting: c: " + convertFloatToString(c));
    ThingSpeak.setField(1,c);
    ThingSpeak.writeFields(SETTINGS_THINGSPEAK_CHANNEL, SETTINGS_THINGSPEAK_KEY);
  } 
  else {
    Serial.println("dht read error");
  }
  delay(60000);
}

void readTemp(void) {
  tempsensor.shutdown_wake(0);   // Don't remove this line! required before reading temp
  c = tempsensor.readTempC();
  // f = c * 9.0 / 5.0 + 32;
  f = (c * 1.8) + 32;
  tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
  delay(250);
}

void setupTempSensor(void) {
  delay(100);
  int started = 0;
  started = tempsensor.begin(0x18);
  if (!started) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
}

void setupOled(void) {
  ui.setTargetFPS(30);

  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // // Add frames
  // ui.setFrames(frames, frameCount);

  // // Add overlays
  // ui.setOverlays(overlays, overlaysCount);

  // Inital UI takes care of initalising the display too.
  ui.init();

  display.flipScreenVertically();

  display.setFont(Orbitron_Light_Plain_12);
  display.drawString(0,0, "OLED Setup");
  display.display();
}

String convertFloatToString(float n) {
  char buf2[16];
  return dtostrf(n, 5, 2, buf2);
}

void setupWifi(void) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}




void setup() {
  Serial.begin(9600);
  Serial.println("Booting");

  setupWifi();

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready OTA");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  setupTempSensor();
  setupOled();
  delay(2000);
  ThingSpeak.begin(client);
  delay(100);
}

int counter = 120;

void loop() {
  counter++;
  // 1200 * 100 - this should be about 2 min, but not sure how long ArduinoOTA.handle() takes...
  readTemp();
  display.clear();
  display.setFont(Just_Another_Hand_Plain_48);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  int fInt = round(f);
  String outS = String(fInt);
  outS += "°F";
  display.drawString(64,16, outS);
  display.display();
  
  
  if (counter > 120) {
    Serial.println("posting value...");
    postValues();
    counter = 0;
  }
  ArduinoOTA.handle();
  delay(1000);
}
