/**
 *  Internet of Things style Temperature and Humidity sensor
 *
 *  Uses ESP8266 microcontroller - with wifi
 *    Device: LOLIN D1 Mini (Wemos clone)
 *  Sensor: SHT30 LOLIN board for temperature and humidity
 *
 *  Reads sensor and uploads values to MQTT for processing
 *  Uses Crandall's EEPROM Menu system for storing configurations
 * 
 *  @author Aaron S. Crandall <acrandal@gmail.com>, 2020
 *  @copyright 2020
 */

#include <ESP8266WiFi.h>
#include <WEMOS_SHT3X.h>            // SHT30 Temperature & Humidity
#include <PubSubClient.h>           // MQTT client

#include "eepromMenu.h"


ESP8266_EEPROM_Configs g_configs;   // From eepromMenu.h
SHT3X sht30(0x45);                  // Temp & Humid I2C sensor

WiFiClient espClient;
PubSubClient client(espClient);     // MQTT client

#define MSG_BUFFER_SIZE  (100)
char msg[MSG_BUFFER_SIZE];

#define SAMPLE_INTERVAL_MILLIS 60000  // 60k millis = 1 minute
#define MQTT_TOPIC "IoT/Data"
unsigned long lastMsgMillis = 0 - SAMPLE_INTERVAL_MILLIS;


bool g_light_on = false;

// ** Flip the LED state ******************************************
void toggle_light() {
  if( g_light_on ) {
    digitalWrite(BUILTIN_LED, HIGH);
    g_light_on = false;
  } else {
    digitalWrite(BUILTIN_LED, LOW);
    g_light_on = true;
  }
}

// ** Initialize hardware systems *********************************
void init_hardware() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  
  Serial.begin(115200);             // Init serial
  while(!Serial) { delay(100); }
  delay(1000);                      // Give serial time to settle

  Serial.println("\nHardware initialized.");
}


// ** Setup, initialize, and connect to the wifi network ********************************
void wifi_connect(char* ssid, char* password) {
  Serial.print("# Initializing Wifi -> ");
  WiFi.begin(ssid, password);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(250);
    toggle_light();
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}


// ** Reconnect to the MQTT server as needed ***************************************
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";                   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {               // Attempt to connect
      Serial.println("MQTT client connected.");

      // Once connected, publish an announcement...
      // snprintf (msg, MSG_BUFFER_SIZE, "IoT Device connected -- %s", clientId.c_str());
      // client.publish("outTopic", msg);
      snprintf (msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\":\"%s\", \"value\":\"%s\"}", "status", g_configs.get_DeviceLocation(), "connected");
      client.publish(MQTT_TOPIC, msg);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      for(int i = 0; i < 20; i++) {                     // Wait 5 seconds before retrying
        delay(250);
        toggle_light();
      }
    }
  }
}


// ** One off setup at boot **************************************************************
void setup() {
  init_hardware();

  g_configs.init();           // Read configs from EEPROM - provide serial menu to edit
  wifi_connect(g_configs.get_SSID(), g_configs.get_Password());
  randomSeed(micros());                                 // Varies by I/O jitter

  client.setServer(g_configs.get_MQTTServer(), 1883);   // 1883 is the default MQTT port

  // ** Start main operations
  Serial.println("Setup complete, beginning normal operations -->");
}


// ** Repeated loop called when loop ends *************************************************
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgMillis > SAMPLE_INTERVAL_MILLIS) {
    // toggle_light();                  // Enable to toggle with every sample (user feedback)
    
    lastMsgMillis = now;                // Snag current time for next delay

    if(sht30.get()==0) {
      float temperature_c = sht30.cTemp;
      float temperature_f = sht30.fTemp;
      float humidity_pct = sht30.humidity;
      char str_temp[10];

      /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
      dtostrf(temperature_c, 4, 2, str_temp);

      // Send temperature in JSON format
      snprintf (msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\":\"%s\", \"value\":%s}", "temperature", g_configs.get_DeviceLocation(), str_temp);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(MQTT_TOPIC, msg);

      // Send humidity in JSON format
      dtostrf(humidity_pct, 4, 2, str_temp);
      snprintf (msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\":\"%s\", \"value\":%s}", "humidity", g_configs.get_DeviceLocation(), str_temp);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(MQTT_TOPIC, msg);
    } else {
      Serial.println("SHT30 Error on read");
    }
  }
}
