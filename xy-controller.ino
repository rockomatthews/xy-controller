#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
#include <WiFi.h>
#define SerialMon Serial
#include <AppleMIDI_Debug.h>
#include <AppleMIDI.h>  // Add the AppleMIDI library

char ssid[] = "ATT4K"; // your network SSID (name)
char pass[] = "passpasspass"; // your network password (use for WPA, or use as key for WEP)

// TFT display and touchscreen pins
#define TFT_CS   5
#define TFT_DC   4
#define TFT_RST  2
#define TS_CS    15

const int DEBOUNCE_DELAY = 70; // the debounce time; increase if the output flickers

// The display uses hardware SPI, plus TFT_RST and TFT_DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 ts = Adafruit_STMPE610(TS_CS);

// global variables to keep track of the previous position
int previous_x = -1;
int previous_y = -1;
unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long t0 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void setup() {
  Serial.begin(115200);
  Serial.print("working");
  tft.begin();
  tft.fillScreen(HX8357_BLACK); // Set the background to black
  Serial.println();
  
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  
  Serial.print("working");
  DBG_SETUP(115200);
  Serial.print("Booting");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Establishing connection to WiFi..");
  }
  Serial.print("Connected to network");
  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.println(F("Add device named Arduino with Host"));
  Serial.println(WiFi.localIP());
  Serial.println(F("Port"));
  Serial.println(AppleMIDI.getPort());
  Serial.println(F("(Name"));
  Serial.println(AppleMIDI.getName());
  Serial.println(F(")"));
  Serial.println(F("Select and then press the Connect button"));
  Serial.println(F("Then open a MIDI listener and monitor incoming notes"));
  Serial.println(F("Listen to incoming MIDI commands"));

  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });
}

void loop() {
  MIDI.read();

  if (ts.touched()) {
    Serial.println("Screen is pressed");
    TS_Point p = ts.getPoint();
    int cc11 = map(p.x, 0, tft.width(), 0, 127);
    int cc10 = map(p.y, 0, tft.height(), 0, 127);

    // send MIDI messages
    MIDI.sendControlChange(66, 127, 5);
    MIDI.sendControlChange(67, cc11, 5);
    MIDI.sendControlChange(68, cc10, 5);

    // Draw blue circle at touch point
    tft.fillCircle(p.x, p.y, 10, HX8357_BLUE);

    // Draw a tail from the previous position to the current position
    if (previous_x != -1 && previous_y != -1) {
      tft.drawLine(previous_x, previous_y, p.x, p.y, HX8357_BLUE);
    }

    // update the previous position
    previous_x = p.x;
    previous_y = p.y;   

  } else {
    MIDI.sendControlChange(66, 0, 5);
    // reset previous position
    previous_x = -1;
    previous_y = -1;
  }
}
