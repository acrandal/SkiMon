/**
 *  SkiMon Hand Console Control and Status Device
 * 
 *  @author Aaron S. Crandall <crandall@gonzaga.edu>
 *  @copyright 2021
 * 
 *  ESP8266 / Wemos D1 Mini based device for the GU SkiMon Project
 */


#define DEVICE_LED_PIN = LED_BUILTIN;
int red_button_pin = D7;
int green_button_pin = D5;

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(115200);
  // initialize the digital pin as an output.
  pinMode(DEVICE_LED_PIN, OUTPUT);
  pinMode(red_button_pin, INPUT_PULLUP);
  pinMode(green_button_pin, INPUT_PULLUP);
  digitalWrite(DEVICE_LED_PIN, LOW);
  delay(1000);
  digitalWrite(DEVICE_LED_PIN, HIGH);
  delay(1000);
}

// the loop routine runs over and over again forever:
void loop() {
  if( digitalRead(red_button_pin) == LOW ) {
    digitalWrite(DEVICE_LED_PIN, LOW);
    Serial.println("A red button is pressed");
  } else if( digitalRead(green_button_pin) == LOW ) {
    digitalWrite(DEVICE_LED_PIN, LOW);
    Serial.println("A green button is pressed");
  } else {
    digitalWrite(led, HIGH);
  }
  delay(50);
}
