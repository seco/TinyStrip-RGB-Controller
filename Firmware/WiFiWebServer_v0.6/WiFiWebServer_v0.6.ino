/*
  Program Name: WiFi LED Strip Controller
  Author: Jeff Murchison
  Contact: jeff@jeffmurchison.com
  Date: June 2016
  Version: 0.6
  Description:

  This sketch allows an ESP-12 wifi module to act as an LED strip controller for an
  RGB LED strip. The controller accepts basic GET requests in order to set LED values.

  License:

  Creative Commons Attribution-ShareAlike 3.0 Unported License.
  http://creativecommons.org/licenses/by-sa/3.0/

  If you wish to license this code under a commercial license, please contact me.

  v0.6
  -----
  - Added rotary encoder support
  - Added ArduinoOTA support


  To-Do
  -----
  - Ignore chrome prefetch requests (somehow)
  - Add timeout for client sending data back?
  - Allow encoder changes during wifi connection (change LED blink delays to blink w/o delay type)
  - Switch to MQTT?

*/


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "config.h"       // Configure WiFi connection details in config.h
#include <Encoder.h>

Encoder myEnc(D1, D2);

long oldPosition  = 0;
boolean isButtonPressed = false;
long lastUpdateMillis = 0;
int encoderMultiplier = 20; // Multiplier value for encoder readings (default 20)

String valStr;  // String for storing RGB values before they're converted to ints

long rVal = 0;      // Variables for storing RGB values
long gVal = 0;
long bVal = 0;

int rLoc;       // Variables for storing char location of 'R' 'G' and 'B' in GET request
int gLoc;
int bLoc;

int ledR = 14;        // Variables for storing LED pin numbers
int ledG = 12;
int ledB = 16;
int ledStatus = 2;

WiFiServer server(80);      // Create an instance of the web server on port 80

void handleKey() {
  isButtonPressed = true;
}

void setup() {
  Serial.begin(115200);     // Serial for debugging
  delay(10);
  pinMode(D4, INPUT_PULLUP);                // Setup pin for encoder pushbutton
  attachInterrupt(D4, handleKey, RISING);   // Attach interrupt to encoder pushbutton pin

  pinMode(ledR, OUTPUT);    // Set up our RGB transistor pins
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(ledStatus, OUTPUT);

  connectWifi();

}


void loop() {

  WiFiClient client = server.available();

  if (!client) {    // Check if a client has connected. If not, return nothing.
    encoderCheck(); // Check the rotary encoder
    wifiCheck(); // Check if device is still connected
    ArduinoOTA.handle();
    return;
  }

  Serial.println("---------");
  Serial.println("New Client Connection");
  while (!client.available()) {     // Wait until the client sends some data
    delay(1);
    encoderCheck(); // Check the rotary encoder
    wifiCheck(); // Check if device is still connected
    ArduinoOTA.handle();
  }

  String req = client.readStringUntil('\r');  // Read the first line of the request
  Serial.print("Client IP: ");
  Serial.println(client.remoteIP());         // Print client IP address

  Serial.print("Raw Request: ");    // Print raw GET request
  Serial.println(req);

  req.replace("GET /", "");         // Remove unnecessary parts of GET request
  req.replace(" HTTP/1.1", "");

  client.flush();

  if (req.indexOf("favicon.ico") == -1) {   // Don't bother parsing the request if it's for the favicon

    Serial.print("Stripped Request: ");     // Print stripped GET request
    Serial.println(req);

    //Serial.print("Individual Values: ");

    // Ensure GET request contains the proper data in the proper places
    // Proper format is R0000G1111B2222 with max values of 1023
    // Each channel needs at least one digit, and should be in order (R first, G second, B last)

    rLoc = req.indexOf('R');
    gLoc = req.indexOf('G');
    bLoc = req.indexOf('B');

    //Serial.println(rLoc);
    //Serial.println(gLoc);
    //Serial.println(bLoc);

    // Check structure of the GET request. GET request must meet the following criteria:
    // 1. Has characters R, G, and B, one for each channel.
    // 2. R character comes first, G character comes second, B character comes third.
    // 3. Each character has at least one digit behind it.
    // Some of these restrictions can probably removed and this code section could be more efficient.

    if (req.length() >= 6) {    // Ensure request is at least 6 characters - RGB and at least one digit for each
      if (rLoc == 0) {    // Ensure 'R' is in the request
        if (isDigit(req.charAt(rLoc + 1)) == true) {  // Ensure digit after 'R' is number
          if (gLoc > rLoc) {   // Ensure 'G' is in the request and after 'R'
            if (isDigit(req.charAt(gLoc + 1)) == true) {  // Ensure digit after 'G' is number
              if (bLoc > gLoc) {   // Ensure 'B' is in the request and after 'G'
                if (isDigit(req.charAt(bLoc + 1)) == true) {  // Ensure digit after 'B' is number

                  // Get R value
                  // Serial.println("R values: ");
                  valStr = "";
                  for (int r = rLoc + 1; r < gLoc; r++) {
                    valStr = (valStr + req.charAt(r));
                    //Serial.println(valStr);
                  }
                  rVal = valStr.toInt();              // Change string to int
                  if (rVal > 1023) {                  // If value > 1023, drop it to 1023 to avoid issues
                    rVal = 1023;
                  }
                  Serial.print("R:");
                  Serial.println(rVal);

                  // Get G value
                  // Serial.println("G values: ");
                  valStr = "";
                  for (int g = gLoc + 1; g < bLoc; g++) {
                    valStr = (valStr + req.charAt(g));
                    // Serial.println(valStr);
                  }
                  gVal = valStr.toInt();              // Change string to int
                  if (gVal > 1023) {                  // If value > 1023, drop it to 1023 to avoid issues
                    gVal = 1023;
                  }
                  Serial.print("G:");
                  Serial.println(gVal);

                  // Get B value
                  // Serial.println("B values: ");
                  valStr = "";
                  for (int b = bLoc + 1; b < req.length(); b++) {
                    valStr = (valStr + req.charAt(b));
                    //Serial.println(valStr);
                  }
                  bVal = valStr.toInt();              // Change string to int
                  if (bVal > 1023) {                  // If value > 1023, drop it to 1023 to avoid issues
                    bVal = 1023;
                  }
                  Serial.print("B:");
                  Serial.println(bVal);
                }
              }
            }
          }
        }
      }
    }

    analogWrite(ledR, rVal);   // Change R pin value
    analogWrite(ledG, gVal);   // Change G pin value
    analogWrite(ledB, bVal);   // Change B pin value
  }

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nR: ";
  s += rVal;    // Show R value
  s += "\nG: ";
  s += gVal;    // Show G value
  s += "\nB: ";
  s += bVal;    // Show B value
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed

}

// For the encoder, track only the difference in value changed


void encoderCheck() {
  long newPosition = myEnc.read();
  long difference = newPosition - oldPosition;

  if (newPosition != oldPosition) {

    if (newPosition <= 0) {   // Check if encoder is going past 0
      newPosition = 0;
      myEnc.write(0);
      rVal = 0;
      gVal = 0;
      bVal = 0;
    }
    else if ((newPosition * encoderMultiplier) >= 1023) {
      //Serial.println("Over 1023");
      //Serial.println(ceil(newPosition/encoderMultiplier));
      myEnc.write(52);
      rVal = 1023;
      gVal = 1023;
      bVal = 1023;
    }
    else {
      rVal = rVal + (difference * encoderMultiplier);
      gVal = gVal + (difference * encoderMultiplier);
      bVal = bVal + (difference * encoderMultiplier);
    }

    Serial.print("Old position: ");
    Serial.println(oldPosition);
    Serial.print("New position: ");
    Serial.println(newPosition);
    oldPosition = newPosition;

    analogWrite(ledR, rVal);   // Change R pin value
    analogWrite(ledG, gVal);   // Change G pin value
    analogWrite(ledB, bVal);   // Change B pin value

    Serial.print("Encoder reading: ");
    Serial.println(newPosition);
    Serial.print("R Val: ");
    Serial.println(rVal);
    Serial.print("G Val: ");
    Serial.println(gVal);
    Serial.print("B Val: ");
    Serial.println(bVal);
  }
  // software debounce
  if (isButtonPressed && millis() - lastUpdateMillis > 50) {
    isButtonPressed = false;
    lastUpdateMillis = millis();
    if (rVal > 0 || gVal > 0 || bVal > 0) {
      analogWrite(ledR, 0);
      analogWrite(ledG, 0);
      analogWrite(ledB, 0);
      myEnc.write(0);
    }
    else {
      analogWrite(ledR, 1023);
      analogWrite(ledG, 1023);
      analogWrite(ledB, 1023);
      myEnc.write(52);
    }
  }
}

void wifiCheck() {
  // Check to see if we're still connected to the wifi AP
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconncted from AP. Reconnecting...");
    reconnectWifi();
  }

}

void connectWifi() {

  // Connect to WiFi network
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);        // Wifi station mode (connect to wifi network)
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {   // While the device isn't connected to a wifi network
    Serial.print(".");
    digitalWrite(ledStatus, HIGH);   // Blink the Wifi status LED
    delay(250);
    digitalWrite(ledStatus, LOW);
    delay(250);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(ledStatus, HIGH);      // Set wifi status LED HIGH

  server.begin();                     // Start the server
  Serial.println("Server started");
  Serial.println(WiFi.localIP());     // Print the server IP address
  {}

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("LED-Kitchen");
  ArduinoOTA.setPassword((const char *)"123456");

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

}


void reconnectWifi() {
  server.stop();
  delay(1);
  connectWifi();
}


