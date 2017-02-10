/*
  Program Name: WiFi LED Strip Controller
  Author: Jeff Murchison
  Contact: jeff@jeffmurchison.com
  Date: June 2016
  Version: 0.4
  Description:

  This sketch allows an ESP-12 wifi module to act as an LED strip controller for an
  RGB LED strip. The controller accepts basic GET requests in order to set LED values.

  License:

  Creative Commons Attribution-ShareAlike 3.0 Unported License.
  http://creativecommons.org/licenses/by-sa/3.0/

  If you wish to license this code under a commercial license, please contact me.

*/


#include <ESP8266WiFi.h>
#include "config.h"       // Configure WiFi connection details in config.h

String valStr;  // String for storing RGB values before they're converted to ints

long rVal;      // Variables for storing RGB values
long gVal;
long bVal;

int rLoc;       // Variables for storing char location of 'R' 'G' and 'B' in GET request
int gLoc;
int bLoc;

int ledR = 12;        // Variables for storing LED pin numbers
int ledG = 14;
int ledB = 16;
int ledStatus = 2;

WiFiServer server(80);      // Create an instance of the web server on port 80

void setup() {
  Serial.begin(115200);     // Serial for debugging
  delay(10);

  pinMode(ledR, OUTPUT);    // Set up our pins
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(ledStatus, OUTPUT);

  connectWifi();

}


void loop() {

  WiFiClient client = server.available();
  if (!client) {    // Check if a client has connected. If not, return nothing.
    return;
  }

  Serial.println("---------");
  Serial.println("New Client Connection");
  while (!client.available()) {     // Wait until the client sends some data
    delay(1);
  }

  String req = client.readStringUntil('\r');  // Read the first line of the request
  Serial.print("Client IP: ");
  Serial.println(client.remoteIP());          // Print client IP address

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
                  Serial.println("R values: ");
                  valStr = "";
                  for (int r = rLoc + 1; r < gLoc; r++) {
                    valStr = (valStr + req.charAt(r));
                    Serial.println(valStr);
                  }
                  rVal = valStr.toInt();              // Change string to int
                  if (rVal > 1023) {                  // If value > 1023, drop it to 1023 to avoid issues
                    rVal = 1023;
                  }
                  Serial.print("R:");
                  Serial.println(rVal);

                  // Get G value
                  Serial.println("G values: ");
                  valStr = "";
                  for (int g = gLoc + 1; g < bLoc; g++) {
                    valStr = (valStr + req.charAt(g));
                    Serial.println(valStr);
                  }
                  gVal = valStr.toInt();              // Change string to int
                  if (gVal > 1023) {                  // If value > 1023, drop it to 1023 to avoid issues
                    gVal = 1023;
                  }
                  Serial.print("G:");
                  Serial.println(gVal);

                  // Get B value
                  Serial.println("B values: ");
                  valStr = "";
                  for (int b = bLoc + 1; b < req.length(); b++) {
                    valStr = (valStr + req.charAt(b));
                    Serial.println(valStr);
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
}


void reconnectWifi() {
  server.stop();
  delay(1);
  connectWifi();
}


