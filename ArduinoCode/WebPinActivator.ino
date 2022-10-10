// Shamelessly stolen/adapted from:
// https://roboticadiy.com/esp8266-make-your-own-led-control-web-server-in-arduino-ide/

#include <WiFiNINA.h>

char ssid[] = "XXXXXXXX";         //  your network SSID (name) between the " "
char pass[] = "xxxxxxxx";         // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
WiFiServer server(80);            //server socket

WiFiClient client = server.available(); // Listen for incoming clients

// Variable to store the HTTP request
String header;

// These variables store current output state of LED
String pinState = "off";
String counter = "5";

// Assign output variables to GPIO pins
int servoPin = 2;

//////////////// Serial timing section //////////////
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
//////////////// Serial timing section //////////////

//////////////// Pin timeout section ////////////////
unsigned long startTime;
unsigned long interval = 15000;
//////////////// Pin timeout section ////////////////

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(servoPin, OUTPUT);
  // Set outputs to LOW
  digitalWrite(servoPin, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) { // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = ""; // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) { // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c); // print it out the serial monitor
        header += c;
        if (c == '\n') { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display state of pin servoPin
            if (header.indexOf("GET /on") >= 0) {
              Serial.println("Servo Pin is on");
              startTime = millis();
              pinState = "on";
              digitalWrite(servoPin, HIGH);
            }

            // Handle setting all the different int/string milli/sec bits
            counter = interval / 1000;
            String metaRefresh = counter;

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"refresh\" content=\"" + metaRefresh + ";url=/\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".buttonRed { background-color: #ff0000; border: none; color: white; padding: 16px 40px; border-radius: 60%;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".buttonOff { background-color: #77878A; border: none; color: white; padding: 16px 40px; border-radius: 70%;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>This button Turns on the water for a few moments</h1>");

            // Display current state, and ON button for GPIO 2
            client.println("<form action=\"on\" method=\"get\">");
            client.println("<label for=\"interval\">Seconds(between 1 and 60):</label>");
            client.println("<input type=\"number\" id=\"interval\" name=\"interval\" min=\"1\" max=\"60\">");


            // If the pinState is off, it displays the OFF button
            if (pinState == "off") {
              client.println("<input type=\"submit\" class=\"button buttonOff\" value=\"OFF\">");
            } else {
              client.println("<input type=\"submit\" class=\"button buttonRed\" value=\"ON\">");
            }
            client.println("</form>");


            int randomReading = analogRead(A1);
            client.print("Random reading from analog pin to verify page refresh: ");
            client.print(randomReading);

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  // If pin is on, check for timeout, turn pin off if met
  if (pinState == "on") {
    if (millis() - startTime >= interval)
    {
      Serial.println("Timeout of " + counter + " sec met, setting Servo Pin to off");
      digitalWrite(servoPin, LOW);
      pinState = "off";
    }
  }
}
