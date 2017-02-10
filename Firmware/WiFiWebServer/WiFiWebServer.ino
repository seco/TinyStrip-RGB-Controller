/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

const char* ssid = "Zombie Defense Network";    // Wifi network SSID
const char* password = "rule#2doubletap";              // Wifi password

String ValStr;  // String for storing RGB values before they're converted to ints

long rVal;     // Ints for storing the RGB values
long gVal;
long bVal;


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);     // Serial for debugging
  delay(10);

  pinMode(12, OUTPUT);      // Set up our pins
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);

  //digitalWrite(2, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

//WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(15, HIGH);
    delay(250);
    digitalWrite(15, LOW); 
    delay(250);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(15, HIGH);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("---------");
  Serial.println("New Client Connection");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.print("Client IP: ");
  Serial.println(client.remoteIP());

  Serial.print("Raw Request: ");
  Serial.println(req);

  req.replace("GET /", "");
  req.replace(" HTTP/1.1", "");

  client.flush();


  if (req.indexOf("favicon.ico") == -1) {   // Don't bother parsing the request if it's for the favicon

    Serial.print("Stripped Request: ");
    for (int f = 0; f <= 15; f++) {
      Serial.print(req.charAt(f));
    }
    Serial.println("");
    Serial.print("Individual Values: ");

    // Ensure GET request contains the proper data in the proper places 
    // Proper format is R000G111B222 


    /*
     * 
     * This needs to be rewritten to accommodate 1 and 2-digit values.
     * 
     * Check that R G B are there, and that there's at least 1 number behind them
     * Find character locations for R G B
     * 
     * 
     */


    ValStr = "";

    if (req.charAt(0) == char('R')) {       // If R is the 1st character
      //Serial.println("R found");
      if (req.charAt(4) == char('G')) {     // If G is the 5th character
        //Serial.println("G found");
        if (req.charAt(8) == char('B')) {   // If B is the 9th character
          //Serial.println("B found");

          ValStr = req.charAt(1);             // Add 1st digit for R to string
          ValStr = (ValStr + req.charAt(2));  // Add 2nd digit for R to string
          ValStr = (ValStr + req.charAt(3));  // Add 3rd digit for R to string
          rVal = ValStr.toInt();              // Change string to int
          if (rVal > 255) {                   // If value > 255, drop it to 255 to avoid issues
            rVal = 255;
          }

          Serial.print("R:");
          Serial.print(rVal);

          ValStr = req.charAt(5);             // Add 1st digit for G to string
          ValStr = (ValStr + req.charAt(6));  // Add 2nd digit for G to string
          ValStr = (ValStr + req.charAt(7));  // Add 3rd digit for G to string
          gVal = ValStr.toInt();              // Change string to int
          if (gVal > 255) {                   // If value > 255, drop it to 255 to avoid issues
            gVal = 255;
          }

          Serial.print(" | G:");
          Serial.print(gVal);

          ValStr = req.charAt(9);              // Add 1st digit for B to string
          ValStr = (ValStr + req.charAt(10));  // Add 2nd digit for B to string
          ValStr = (ValStr + req.charAt(11));  // Add 3rd digit for B to string
          bVal = ValStr.toInt();               // Change string to int
          if (bVal > 255) {                    // If value > 255, drop it to 255 to avoid issues
            bVal = 255;
          }

          Serial.print(" | B:");
          Serial.println(bVal);


        }
      }
    }

    analogWrite(12, rVal);   // Change R pin value
    analogWrite(13, gVal);   // Change G pin value
    analogWrite(14, bVal);   // Change B pin value
  }

  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nR:  ";
  s += rVal;    // Show R value
  s += "\nG:";
  s += gVal;    // Show G value
  s += "\nB:";
  s += bVal;    // Show B value
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

