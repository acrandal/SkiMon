#include <ESP8266WebServer.h>
//#include <U8g2lib.h>
 
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4);
 
const char *ssid = "Alexahome";
const char *password = "loranthus";
 
ESP8266WebServer server(80);
 
void handleSentVar() {
  Serial.println("handleSentVar function called...");
  if (server.hasArg("sensor_reading")) { // this is the variable sent from the client
    Serial.println("Sensor reading received...");
 
    int readingInt = server.arg("sensor_reading").toInt();
  //  char readingToPrint[5];
   // itoa(readingInt, readingToPrint, 10); //integer to string conversion for OLED library
   // u8g2.firstPage();
   // u8g2.drawUTF8(0, 64, readingToPrint);
   // u8g2.nextPage();
 
    Serial.print("Reading: ");
//    Serial.println(readingToPrint);
    Serial.println(readingInt);
    Serial.println();
    server.send(200, "text/html", "Data received");
  }
}
 
void setup() 
{
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
 
 // u8g2.begin();
//  u8g2.setFont(u8g2_font_logisoso62_tn);
 // u8g2.setFontMode(0);    // enable transparent mode, which is faster
 
  WiFi.softAP(ssid, password);
 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/data/", HTTP_GET, handleSentVar);
  server.begin();
  Serial.println("HTTP server started");
 
}
 
void loop() {
  server.handleClient();
}
