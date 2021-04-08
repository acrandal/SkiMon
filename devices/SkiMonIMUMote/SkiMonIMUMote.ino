/**
 *  SkiMon Project Inertial Measurement Unit Mote (IMU Mote)
 *
 *  Uses ESP8266 microcontroller - with wifi
 *    Device: LOLIN D1 Mini (Wemos clone)
 *  Sensor: Pololu Altimu10-v5
 *
 *  Reads sensor and uploads values to MQTT for processing
 *  Uses Crandall's EEPROM Menu system for storing configurations
 * 
 *  @author Aaron S. Crandall <crandall@gonzaga.edu>
 *  @copyright 2021
 */

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>           // MQTT client
#include <Adafruit_LSM6DS33.h>      // LSM6 driver from Adafruit
#include <LIS3MDL.h>                // LIS3MDL - Magnetometer from Pololu

#include "eepromMenu.h"

ESP8266_EEPROM_Configs g_configs;   // From eepromMenu.h

WiFiClient espClient;
PubSubClient client(espClient);     // MQTT client


#define MSG_BUFFER_SIZE  (200)
char msg[MSG_BUFFER_SIZE];

//#define SAMPLE_INTERVAL_MILLIS 60000  // 60k millis = 1 minute
#define SAMPLE_INTERVAL_MILLIS 50  // 1/20 second (20 Hz)
#define MQTT_TOPIC "skimon"
unsigned long lastMsgMillis = 0 - SAMPLE_INTERVAL_MILLIS;


unsigned long startClockMillis;
int clockSamplesCount;
unsigned long lastSampleMillis;
unsigned long endSampleMillis;
int loopMicroseconds;
#define SAMPLE_FREQUENCY 50           // In Hz ******************************************************

Adafruit_LSM6DS33 lsm6ds33;


// Magenetomter resources
LIS3MDL mag;
unsigned long lastMagMillis = 0;



bool g_light_on = false;

// ** Flip the LED state ******************************************
void turn_light_on() { digitalWrite(BUILTIN_LED, LOW); }
void turn_light_off() { digitalWrite(BUILTIN_LED, HIGH); }

void toggle_light() {
  if( g_light_on ) {
//    digitalWrite(BUILTIN_LED, HIGH);
    turn_light_off();
    g_light_on = false;
  } else {
//    digitalWrite(BUILTIN_LED, LOW);
    turn_light_on();
    g_light_on = true;
  }
}

// ** Initialize hardware systems *********************************
void init_hardware() {
  Wire.begin();
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  
  Serial.begin(115200);             // Init serial
  //while(!Serial) { d8elay(100); }
  delay(1000);                      // Give serial time to settle

//  // ** Initialize the LSM6 IMU sensor **
//  if (!imu.init())
//  {
//    Serial.println("Failed to detect and initialize IMU!");
//    while (1);
//  }
//  imu.enableDefault();
//  imu.setAccScale(ACC4g);

  // ** Initialize the LSM6 IMU sensor **
  if (!lsm6ds33.begin_I2C(0x6B)) {
    Serial.println("Failed to find LSM6DS33 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("LSM6DS33 Found!");

  lsm6ds33.setAccelRange(LSM6DS_ACCEL_RANGE_8_G);
  // lsm6ds33.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm6ds33.setAccelDataRate(LSM6DS_RATE_208_HZ);
  lsm6ds33.setGyroDataRate(LSM6DS_RATE_208_HZ);

  lsm6ds33.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds33.configInt2(false, true, false); // gyro DRDY on INT2

  // Magnetometer
  if (!mag.init())
  {
    Serial.println("Failed to detect and initialize magnetometer!");
    while (1);
  }
  Serial.println("Magenetometer initialized");
  mag.enableDefault();      // Set to +/- 4 gauss

  Serial.println("\nHardware initialized.");
}


// ** Setup, initialize, and connect to the wifi network ********************************
void wifi_connect(char* ssid, char* password) {
  Serial.print("# Initializing Wifi -> ");
  WiFi.mode(WIFI_STA);                        // Required for wireless to wireless MQTT (for some reason).
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

    String clientId =  String(g_configs.get_DeviceLocation());
    //String clientId = "ESP8266Client-";                   // Create a random client ID
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    Serial.print("MQTT Client ID: ");
    Serial.println(clientId);
    
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


// ** Read IMU for accel & gyro values - publish to MQTT *****************************
void doIMUSample() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds33.getEvent(&accel, &gyro, &temp);

  //Serial.print(temp.temperature);
  //Serial.print(",");
  
  snprintf(msg, MSG_BUFFER_SIZE, "A: %6f %6f %6f    G: %6f %6f %6f",
    accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
    gyro.gyro.x, gyro.gyro.y, gyro.gyro.z);
  //Serial.println(msg);

  // ** Do Accelerometer
  snprintf(msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\": \"%s\", \"value\": {\"x\": %6f, \"y\": %6f, \"z\": %6f}}",
    "accel",
    g_configs.get_DeviceLocation(),
    accel.acceleration.x,
    accel.acceleration.y,
    accel.acceleration.z
  );
  client.publish(MQTT_TOPIC, msg);
  //Serial.println(msg);  

  // ** Do gyro next
  snprintf(msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\": \"%s\", \"value\": {\"x\": %6f, \"y\": %6f, \"z\": %6f}}",
    "gyro",
    g_configs.get_DeviceLocation(),
    gyro.gyro.x,
    gyro.gyro.y,
    gyro.gyro.z
  );  
  client.publish(MQTT_TOPIC, msg);
  //Serial.println(msg);  
}


// ** ************************************************************************************
void doMagSample() {
  mag.read();

  // ** Do magnetometer publish
  snprintf(msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\": \"%s\", \"value\": {\"x\": %6d, \"y\": %6d, \"z\": %6d}}",
    "mag",
    g_configs.get_DeviceLocation(),
    mag.m.x,
    mag.m.y,
    mag.m.z
  );
  client.publish(MQTT_TOPIC, msg);
  //Serial.println(msg);
}


// ** One off setup at boot **************************************************************
void setup() {
  init_hardware();

  // Sampling rate values and configs
  clockSamplesCount = 0;
  startClockMillis = millis();
  endSampleMillis = millis();

  loopMicroseconds = 1000000 / SAMPLE_FREQUENCY;


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
    turn_light_on();
    reconnect();
    turn_light_off();
  }
  client.loop();

//  unsigned long now = millis();
//  if (now - lastMsgMillis > SAMPLE_INTERVAL_MILLIS) {
//    // toggle_light();                  // Enable to toggle with every sample (user feedback)
//    
//    lastMsgMillis = now;                // Snag current time for next delay
//    //Serial.println("Sample!");
//
//    doIMUSample();
//  }

  doIMUSample();

  if( lastMagMillis + 1000 < millis() ) {
    lastMagMillis = millis();
    doMagSample();
  }

  // Debugging routing for tracking sampling frequency
  clockSamplesCount++;
  
  if( startClockMillis + 1000 < millis() ) {
    //Serial.print("Samp/sec: ");
    //Serial.print(",");
    Serial.print("\t\t\t\t\t\t\t");
    Serial.print(clockSamplesCount);
    Serial.println();
    clockSamplesCount = 0;
    startClockMillis = millis();
  }

  // Calculate delay needed to hit sampling frequency
  int currDelay = (endSampleMillis * 1000 + loopMicroseconds) - (millis() * 1000);
  currDelay = max(currDelay, 0);        // Can't be negative
  delayMicroseconds(currDelay);
  endSampleMillis = millis();
}
