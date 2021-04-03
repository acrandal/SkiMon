/**
 * 
 * 
 * 
 * 
 * 
 * 
 */


#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>           // MQTT client
#include <LSM6.h>

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

void display_status(char* msg) {
  display_status(msg, 0, false);
}

void display_status(char* msg, int delayMS) {
  display_status(msg, delayMS, false);
}

// ****************************************************************
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
void display_settings() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  //display.println(empty_spool_range);
  //display.print("Full:  ");
  //display.println(full_spool_range);
  display.display();
}


// ****************************************************
void display_clear() {
  display.clearDisplay();
  display.display();
}


// ****************************************************
void update_screen(bool got_range) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.setTextSize(1);
  display.println("");
  display.setTextSize(2);

  display.print(" ");
  display.println("%");
  
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
  
  Serial.println("\nHardware initialized.");
}


// *************************************************************************************************** //
void setup() {
  init_hardware();                                                // Setup hardware in/out

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
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
  display_status("MQTT Conn", 1000);

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

//  update_screen(got_range);
  //Serial.println("hi");
  if( is_go_button_pressed() ) {
    Serial.println("Green: Pressed!");
    sendRecordingStatus("start");
  }
  
  if( is_stop_button_pressed() ) {
    Serial.println("Red: Pressed!");
    sendRecordingStatus("stop");
  }

  delay(100);
}
