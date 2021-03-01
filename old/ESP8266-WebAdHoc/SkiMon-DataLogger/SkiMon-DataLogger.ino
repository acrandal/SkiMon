#include <ESP8266WiFi.h>
/*
#include <OneWire.h>
#include <DallasTemperature.h>
 
#define DS18B20 D2
 
OneWire ourWire(DS18B20);
DallasTemperature sensor(&ourWire);
*/

#include <WEMOS_SHT3X.h>

SHT3X sht30(0x45);
 
const char *ssid = "Alexahome";
const char *password = "loranthus";
 
int temp = 0;        // value read from the DS18B20
 
void setup() {
  Serial.begin(115200);
  delay(1000);
 
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() 
{
  // read the temperature value:
  //sensor.requestTemperatures();
  //temp = sensor.getTempCByIndex(0);

  if(sht30.get()==0){
    Serial.print("Temperature in Celsius : ");
    Serial.println(sht30.cTemp);
    Serial.print("Temperature in Fahrenheit : ");
    Serial.println(sht30.fTemp);
    Serial.print("Relative Humidity : ");
    Serial.println(sht30.humidity);
    Serial.println();
  }
  else
  {
    Serial.println("Error!");
  }
  
 
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" °C");
 
  char intToPrint[5];
  itoa(temp, intToPrint, 10); //integer to string conversion for OLED library
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.4.1";
  const int httpPort = 80;
 
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
  // We now create a URI for the request
  String url = "/data/";
  url += "?sensor_reading=";
  url += intToPrint;
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 
  Serial.println();
  Serial.println("Closing connection");
  Serial.println();
  Serial.println();
  Serial.println();
 
  delay(500);
}
