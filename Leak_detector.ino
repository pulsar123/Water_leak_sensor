/*
 *  Smart Water Leak Detector, by Sergey Mashchenko
 *  
 *  For ESP8266 microcontroller. Requires: 
 *   - a water sensor (two metal contacts)
 *   - red and green (or blue) LEDs 
 *   - a microswitch button
 *   - piezo buzzer (like SFM-27) working from DC voltage, plus Step Up Boost Converter (like MT3608), 5V->12V, 
 *      at least 30mA output. Tune the converter to generate 12V when the GPIO pin is set to HIGH.
 *   - NPN transistor (used as a switch), to provide enough current to the piezo buzzer (via the step up voltage converter).
 *      I am using Darlington transistor (TIP120) simply because I have lots of them, but any NPN transistor with
 *      collector voltage rating >5V, collector current rating >60mA, and hFE (at 60 mA) of at least 5, will do.
 *      The base resistor (R1) value can be calculated using the following link, by setting RL=83 Ohm, Vcc=5V, Vi=3.3V,
 *      and using your transistor's hFE value (at collector current 60mA). Make sure the base current (Vi/Rb) is <=12 mA,
 *      or you will damage the GPIO pin of the microcontroller.
 *         https://www.petervis.com/GCSE_Design_and_Technology_Electronic_Products/transistor_base_resistor_calculator/transistor_base_resistor_calculator.html
 *   - a few resistors (voltage divider for the water sensor; limiting resistors for the two LEDs and the base resistor)
 *   -- base resistor: 15k (for TIP120), use the above link to calculate it for your transistor
 *   -- for red/green LEDs: 100...330 Ohm (make sure it is large enough to limit the GPIO current to 12mA, which is
 *       maximum what ESP8266 pins can handle, 3.3V voltage)
 *   -- ~200k for the water sensor
 * 
 *  Uses WiFi to connect to a MQTT server on the local network (which in turn can be integrated with OpenHAB web server, 
 *  for full smart home integration).
 *  
 *  When a water leak is detected (resistance in the water sensor falls within a specified range), piezo alarm goes off, 
 *  the red LED start flushing, and the alarm is sent to MQTT server.
 *  
 *  Pressing the button would disable the alarm, and will disable water sensing for a specified time (DT_QUIET ms). 
 *  If water is still detected after that time, alarm is resumed.
 *  
 *  One can integrate multiple sensors via MQTT/OpenHAB, to do more tasks, e.g.:
 *   - triggering one sensor will make all sensors in the house sound an alarm (until a button is pressed on any of them).
 *     Red LED will flush differently depending on the ID of the sensor which originated the alarm.
 *   - alarm can trigger sending an email
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include "config.h"

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  delay(100);
#endif
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  WiFi_on = 0;
  MQTT_on = 0;

  switch_state = 1 - digitalRead(SWITCH_PIN); // reading the current state of the physical switch
  switch_state_old = switch_state;

  buzzer_state = 0;
  local_alarm = 0;

  green_led = 0;
  red_led = 0;

// Initially LEDs are off, no buzzer:
  digitalWrite(RED_LED_PIN, red_led);
  analogWrite(GREEN_LED_PIN, 0);
  digitalWrite(BUZZER_PIN, buzzer_state);

    #ifdef DEBUG
    delay(10000);
    Serial.println("Starting code");
    #endif


// WiFi / MQTT initialization (non-blocking):
    #ifdef DEBUG
    Serial.println("WiFi mode");
    #endif
  WiFi.mode(WIFI_STA);
    #ifdef DEBUG
    Serial.println("WiFi begin");
    #endif
  WiFi.begin(ssid, password);
    #ifdef DEBUG
    Serial.println("set server mqtt");
    #endif
  client.setServer(mqtt_server, 1883);
    #ifdef DEBUG
    Serial.println("set callback");
    #endif
  client.setCallback(callback);
    #ifdef DEBUG
    Serial.println("Setup done");
    #endif

  t0 = millis();
  t = t0;
  t_switch = 0;
  t_a0 = t0;
  t_red_led = t0;
  t_green_led = t0;
  t_mqtt = t0;
  t_alarm = t0;
  t_buzzer = t0;
#ifdef DEBUG
  t_print = t0;
#endif  
  t_water = t0;
  t_quiet = 0;
  t_bad_sensor = 0;
  mqtt_init = 1;
  mqtt_refresh = 0;
  id_alarm = 0;
  buz_flag = 0;
  bad_sensor = 0;
  red_led_flag = 0;
  bad_sensor_flag = 0;
  bad_sensor_flag_old = 0;

}

void loop() {

// Establishing and re-establishing WiFi and MQTT connections:
  connections();
  
  // Reading / sending MQTT message(s):
  mqtt();

  // Reading the water sensor
  water_sensor();

  // Reading the physical switch state (with debouncing):
  read_switch();

  // Warning signals with the red LED:
  led();

  // Switching the buzzer on or off as needed:
  buzzer();

  // End of the loop cleanup:
  cleanup();

}
