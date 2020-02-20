/* The sensor ID (1...9)
 *  1: Utility room
 * 
 */
#define SENSOR_ID 1

//#define DEBUG
#define QUIET
//#define PRINT_SENSOR

/* Personal info (place the following lines in a separate file, private.h, uncomment all the lines, and replace xxxx with your personal details):
  const char* ssid = "xxx";
  const char* password = "xxxx";
  const char* mqtt_server = "xxxx";
*/
#include "private.h" //  Make sure you create this file first (see the lines above)

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

// The root names for MQTT topics
// To talk to other sensors:
#define ROOT "water_sensor"
// To talk to OpenHAB:
#define ROOT_ID ROOT STRINGIZE(SENSOR_ID)
/*  MQTT topics:
  Incoming:
  ROOT"/alarm" : ID of the external sensor where alarm was triggered; 0 to call off alarm if external sensor button was pressed
  openhab/start  : optional; if "1" is received (the sign that openhab has just re-started), the switch will re-publish its current state

  Outgoing:
  ROOT"/alarm"   : for other sensors;  local sensor ID if local alarm was triggered; 0 if local alarm was cancelled (whether from pressing the button, or if water leak is gone)
  ROOT_ID"/alarm" : for OpenHAB; 1 if local alarm was triggered; 0 if local alarm was cancelled (whether from pressing the button, or if water leak is gone)

  For the "openhab/start" thing to work, one needs to have the OpenHab to publish "1" (followed by "0") in this topic at startup.
  Under Windows this can be accomplished by adding this line before the last line of openhab's start.bat file:

  start /b C:\openHAB2\mqtt_start.bat >nul

  and creating a new file mqtt_start.bat with the following content:

  timeout /t 20 /nobreak
  C:\mosquitto\mosquitto_pub.exe -h 127.0.0.1 -t "openhab/start" -m "1"
  C:\mosquitto\mosquitto_pub.exe -h 127.0.0.1 -t "openhab/start" -m "0"
*/

// Pins used (WEMOS D1 mini)
// (https://wiki.wemos.cc/products:d1:d1_mini)
// Pin used to read the state of the button
const byte SWITCH_PIN = 0;  // D3, neeeds builin pullup resistor
// Pin to operate the buzzer (via optocoupler):
const byte BUZZER_PIN = 16; // D0
const byte RED_LED_PIN = 5; // D1
const byte GREEN_LED_PIN = 4; // D2

// The range of resistance corresponding to water:
const int R_min = 150;
const int R_max = 600; 

// Green LED brightness (0-255):
const byte LED_PWM = 10;

const unsigned long int DT_QUIET = 60000;  // Quiet time after pressing the button, ms
const unsigned long int DT_QUIET_RESET = DT_QUIET + 600000;  // Resetting t_quiet after that many ms, to avoid timer overfill (would happen after ~50 days)
const unsigned long int DT_DEBOUNCE = 100; // Physical switch debounce time in ms

const int N_BEEPS = 2; // Number of short beeps in the alarm sound
const unsigned long int DT1_BUZZER = 250; // Short beeps duration, ms
const unsigned long int DT2_BUZZER = 2000; // Interval between groups of short beeps, ms
const unsigned long int DT1_BAD_SENSOR = 50; // Short beep / LED flush duration if bad (shorted) water sensor, ms
const unsigned long int DT2_BAD_SENSOR = 10000; // Interval between short beep / LED flushs if bad (shorted) water sensor, ms

const unsigned long int DT1_RED_LED = 200; // Short flushes duration, ms, for external alarm
const unsigned long int DT2_RED_LED = 2000; // Interval between groups of short flushes, ms (counted from the end of tghe last flush)
const unsigned long int DT3_RED_LED = 500; // Long flashes duration, ms, for local alarm

const unsigned long int DT_WATER = 10; // Interval between water sensor readings, ms (should be at least 10ms, or it will affect WiFi connection)

#ifdef DEBUG
const unsigned long int DT_PRINT = 500;  // interval between water sensor prints, ms
#endif

//+++++++++++++++++++++++++++++ Normally nothing should be changed below ++++++++++++++++++++++++++++++++++++++++++++++++++

WiFiClient espClient;
PubSubClient client(espClient);
static WiFiUDP udp;

#ifdef DEBUG
char tmp[256];
#endif
byte WiFi_on, MQTT_on;
byte mqtt_init;
byte mqtt_refresh;
byte red_led, green_led;
byte switch_state, switch_state_old;
unsigned long int t, t0, t_switch, t_a0, t_green_led, t_red_led, t_mqtt, t_quiet, t_alarm, t_buzzer;
unsigned long int t_bad_sensor, t_water;
byte bad_sensor_flag, bad_sensor_flag_old; // 1/2: start/end of the short beep; 0 is for the start of the initial flash
byte buzzer_state;
byte external_alarm;  // =1 if alarm was triggered over mQTT
byte local_alarm; // =1 if the alarm was triggered locally (not over MQTT)
byte id_alarm; // Sensor id for the external alarm
byte buz_flag; // Flag used for buzzer
byte bad_sensor; // =1 if water sensor is bad (too small resistance - likely shorted), 0 otherwise
byte red_led_flag;
char buf[20];
#ifdef DEBUG
unsigned long int t_print;
#endif
