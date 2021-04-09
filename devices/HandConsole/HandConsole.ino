/**
 * SkiMon Hand Console
 * 
 * @author Aaron S. Crandall <crandall@gonzaga.edu>
 * @copyright 2021
 * 
 * 
 */


#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>           // MQTT client
#include <LSM6.h>
#include <ArduinoJson.h>
#include <Adafruit_BME280.h>

#include "eepromMenu.h"
#include "Adafruit_SSD1306.h"

#define RED_BUTTON_PIN D7
#define GREEN_BUTTON_PIN D5

int lastGreenButtonState = HIGH;    // Defaulting to unpressed
int lastRedButtonState = HIGH;

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);


ESP8266_EEPROM_Configs g_configs;   // From eepromMenu.h

WiFiClient espClient;
PubSubClient client(espClient);     // MQTT client

#define MSG_BUFFER_SIZE  (200)
char msg[MSG_BUFFER_SIZE];

//#define SAMPLE_INTERVAL_MILLIS 60000  // 60k millis = 1 minute
#define SAMPLE_INTERVAL_MILLIS 50  // 1/20 second (20 Hz)
#define MQTT_TOPIC "skimon"
unsigned long lastMsgMillis = 0 - SAMPLE_INTERVAL_MILLIS;

// JSON materials
StaticJsonDocument<200> doc;

struct UIStatus {
  bool recording = false;
  int diskFullPct = 0;
  bool gps = false;
  bool leftFront = false;
  bool leftMiddle = false;
  bool leftBack = false;
  bool rightFront = false;
  bool rightMiddle = false;
  bool rightBack = false;
  int altitude = 0;
  int temperature = 0;
};

UIStatus uiStatus;

// BME 280 Temperature/Humidity/Pressure/Altitude
Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
unsigned long lastBMEmillis = 0;



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


// ** Generate a display a ski on the OLED ******************************************
void display_ski(int x, int y, boolean isOn[3]) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  int textHeight = 8;
  unsigned char mountain = 30;
  unsigned char skiSectionOff = 7;
  unsigned char skiSectionOn = 8;

  for( int i = 0; i < 4; i++ ) {    // Clear old ski drawing
    display.setCursor(x, y + i * textHeight);
    display.print(" ");
    display.setCursor(x, y + i * textHeight);
    if( i == 0 ) {
      display.write(mountain);
    } else {
      if( isOn[i - 1] ) {
        display.write(skiSectionOn);
      } else {
        display.write(skiSectionOff);
      }
    }
  }
}


// ****************************************************************
void display_status(char* msg) {
  display_status(msg, 0, false);
}

void display_status(char* msg, int delayMS) {
  display_status(msg, delayMS, false);
}

void display_status(char* msg, int delayMS, boolean clearFirst) {
  if(clearFirst) {
    display_clear();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
  }
  display.print(msg);
  display.display();
  delay(delayMS);
}


// ****************************************************************
void display_logo() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Ski   Mon");

  boolean skiStatus[3] = {true, true, true};
  display_ski(52, 5, skiStatus);
  display_ski(58, 5, skiStatus);
  display.display();
}


// ****************************************************
void display_clear() {
  display.clearDisplay();
  display.display();
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

      snprintf (msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\":\"%s\", \"value\":\"%s\"}", "status", g_configs.get_DeviceLocation(), "connected");
      client.publish(MQTT_TOPIC, msg);
      
      client.subscribe("skimon/status");
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


// ** Reading Go Button *******************************************
boolean is_go_button_pressed() {
  boolean is_pressed = false;
  if( digitalRead(GREEN_BUTTON_PIN) == HIGH  && lastGreenButtonState == LOW ) {
    lastGreenButtonState = HIGH;
    is_pressed = false;
  }
  else if( digitalRead(GREEN_BUTTON_PIN) == LOW && lastGreenButtonState == HIGH ) {
    lastGreenButtonState = LOW;
    is_pressed = true;
  }
  return is_pressed;
}


// ** Reading Stop Button *******************************************
boolean is_stop_button_pressed() {
  boolean is_pressed = false;
  if( digitalRead(RED_BUTTON_PIN) == HIGH  && lastRedButtonState == LOW ) {
    lastRedButtonState = HIGH;
    is_pressed = false;
  }
  else if( digitalRead(RED_BUTTON_PIN) == LOW && lastRedButtonState == HIGH ) {
    lastRedButtonState = LOW;
    is_pressed = true;
  }
  return is_pressed;
}


// ** Sending a button press ************************************************************************
void sendRecordingStatus(char* valueMsg) {
  snprintf(msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\": \"%s\", \"value\":\"%s\"}",
    "recording",
    g_configs.get_DeviceLocation(),
    valueMsg
  );
  Serial.println(msg);
  client.publish(MQTT_TOPIC, msg);
}


// ** Initialize hardware systems *********************************
void init_hardware() {
  Wire.begin();

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH);     // HIGH means off - the boards are backwards...
  
  Serial.begin(115200);             // Init serial
  while(!Serial) { delay(100); }
  delay(1000);                      // Give serial time to settle
  
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);

  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1) delay(10);
  }
    
  bme_temp->printSensorDetails();
  bme_pressure->printSensorDetails();
  bme_humidity->printSensorDetails();
  
  Serial.println("\nHardware initialized.");
}


// *************************************************************************************************** //
void statusCallbackHandler(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message:");
  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  // Serial.println();

  char msgC[length];
  for (int i = 0; i < length; i++) {
    msgC[i] = (char)payload[i];
  }

  DeserializationError error = deserializeJson(doc, msgC);
  if (error) {
    Serial.println("Error in deserialization");
    return;
  }

  // Parse out the JSON values and fill up our uiStatus struct
  const char* recording = doc["recording"];
  int diskFull = doc["diskFull"];
  const char* gpsStatus = doc["GPS"];

  int leftFrontTimeout = doc["motes"]["LeftFront"];
  int leftMiddleTimeout = doc["motes"]["LeftMiddle"];
  int leftBackTimeout = doc["motes"]["LeftBack"];
  int rightFrontTimeout = doc["motes"]["RightFront"];
  int rightMiddleTimeout = doc["motes"]["RightMiddle"];
  int rightBackTimeout = doc["motes"]["RightBack"];

  uiStatus.recording = (strcmp(recording, "start") == 0);
  uiStatus.diskFullPct = diskFull;
  uiStatus.gps = (strcmp(gpsStatus, "yes") == 0);
  uiStatus.leftFront = (leftFrontTimeout == 0);
  uiStatus.leftMiddle = (leftMiddleTimeout == 0);
  uiStatus.leftBack = (leftBackTimeout == 0);
  uiStatus.rightFront = (rightFrontTimeout == 0);
  uiStatus.rightMiddle = (rightMiddleTimeout == 0);
  uiStatus.rightBack = (rightBackTimeout == 0);
  uiStatus.altitude = (int)bme.readAltitude(1013.25);

  sensors_event_t temp_event;
  bme_temp->getEvent(&temp_event);
  uiStatus.temperature = (int)temp_event.temperature;
  
}

void dumpUIStatus() {
  Serial.println("Recording: " + String(uiStatus.recording));
  Serial.println("Disk Full: " + String(uiStatus.diskFullPct));
  Serial.println("GPS Lock: " + String(uiStatus.gps));
  Serial.println("Left Front: " + String(uiStatus.leftFront));
  Serial.println("Left Middle: " + String(uiStatus.leftMiddle));
  Serial.println("Left Back: " + String(uiStatus.leftBack));
  Serial.println("Right Front: " + String(uiStatus.rightFront));
  Serial.println("Right Middle: " + String(uiStatus.rightMiddle));
  Serial.println("Right Back: " + String(uiStatus.rightBack));
  Serial.println("Altitude: " + String(uiStatus.altitude));
}


// *************************************************************************************************** //
void refreshSkis() {
  bool isOnLeft[3] = {uiStatus.leftFront, uiStatus.leftMiddle, uiStatus.leftBack};
  bool isOnRight[3] = {uiStatus.rightFront, uiStatus.rightMiddle, uiStatus.rightBack};
  display_ski(0,0,isOnLeft);
  display_ski(6,0,isOnRight);
}

void refreshRecording(int x, int y) {
  display.setCursor(x,y);
  String line = "|Dat ";
  if(uiStatus.recording) {    line += "ON";  }
    else {    line += "OFF";  }
  display.print(line);
}

void refreshGPS(int x, int y) {
  display.setCursor(x,y);
  String line = "|GPS ";
  if(uiStatus.gps) {    line += "YES";  }
    else {    line += "NO";  }
  display.print(line);
}

void refreshDisk(int x, int y) {
  display.setCursor(x,y);
  String line = "|DSK ";
  if(uiStatus.diskFullPct < 10) {
    line += String(" ");
  }
  line += String(uiStatus.diskFullPct);
  line += "%";
  display.print(line);
}

void refreshAltitude(int x, int y) {
  display.setCursor(x,y);
  String line = "|Alt";
  if(uiStatus.altitude < 1000) {
    line += String(" ");
  }
  line += String(uiStatus.altitude);
  display.print(line);
}

void refreshTemperature(int x, int y) {
  display.setCursor(x,y);
  String line = "|Tmp ";
  line += String(uiStatus.temperature);
  display.print(line);
}

// *************************************************************************************************** //
void refreshScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  int leftX = 2 * 6;
  int rowHeight = 10;

  refreshSkis();
  refreshRecording(leftX, 0 * rowHeight);
  refreshGPS(leftX, 1 * rowHeight);
  refreshDisk(leftX, 2 * rowHeight);
  refreshAltitude(leftX, 3 * rowHeight);
  refreshTemperature(leftX, 4 * rowHeight);

  display.display();
}

// *************************************************************************************************** //
void sendBMEreading() {
  // Serial.println("Sending BME280 reading");
  
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);

  snprintf (msg, MSG_BUFFER_SIZE, "{\"type\":\"%s\", \"location\":\"%s\", \"temperature\":%s, \"humidity\":%s, \"altitude\":%s}",
    "environ",
    g_configs.get_DeviceLocation(),
    String(temp_event.temperature, 2).c_str(),
    String(humidity_event.relative_humidity, 2).c_str(),
    String(bme.readAltitude(1013.25), 2).c_str());
  client.publish(MQTT_TOPIC, msg);
}


// *************************************************************************************************** //
void setup() {
  init_hardware();                                                // Setup hardware in/out

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.cp437(true);                        // Use full 256 char 'Code Page 437' font
  display.display();

  display_logo();
  delay(5000);

  display_status("Loading\nconfigs:", 0, true);
  g_configs.init();                                               // Read configs from EEPROM - provide serial menu to edit
  display_status("\n -- Loaded", 1000);
  
  display_status("Connecting\nWifi:", 0, true);
  wifi_connect(g_configs.get_SSID(), g_configs.get_Password());   // Connect to WiFi
  display_status("\nConnected!", 1000);
  
  randomSeed(micros());                                           // Used for name - Varies by I/O jitter

  display_status("Connecting\nMQTT Srvr", 0, true);
  client.setServer(g_configs.get_MQTTServer(), 1883);             // 1883 is the default MQTT port
  display_status("MQTT Conn", 250);
  client.setCallback(statusCallbackHandler);
  client.subscribe("skimon/status");
  display_status("\n-Subscribe", 1000);

  // ** Start main operations
  Serial.println("Setup complete, beginning normal operations -->");
  display_status("Begin Ops.", 2000, true);
}


// *************************************************************************************************** //
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if( is_go_button_pressed() ) {
    Serial.println("Green: Pressed!");
    sendRecordingStatus("start");
  }
  
  if( is_stop_button_pressed() ) {
    Serial.println("Red: Pressed!");
    sendRecordingStatus("stop");
  }

  if( lastBMEmillis + 1000 < millis() ) {
    lastBMEmillis = millis();
    sendBMEreading();
  }

  refreshScreen();

  delay(100);
}
