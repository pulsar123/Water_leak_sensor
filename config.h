/* The sensor ID (1...9)
 *  1: Utility room
 * 
 */
#define SENSOR_ID 1

//#define DEBUG  // Debugging mode. Prints debugging info to the serial interface
//#define QUIET  // Used for debugging purposes. Disables the buzzer
//#define PRINT_SENSOR  // Used when DEBUG is enabled. Prints water sensor values to serial connection at equal intervals (DT_PRINT ms)
//#define BUZZER_TEST // Turns buzzer on, to tune its volume (voltage)

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
  ROOT/alarm : ID of the external sensor where alarm was triggered; 0 to call off alarm if external sensor no longer detect water leak
  ROOT/quiet : if anything is received, pause the alarm (both local and external), for DT_QUIET ms
  openhab/start  : optional; if "1" is received (the sign that openhab has just re-started), the switch will re-publish its current state

  Outgoing:
  ROOT/alarm   : for other sensors;  local sensor ID if local alarm was triggered; 0 if local alarm was cancelled (whether from pressing the button, or if water leak is gone)
  ROOT_ID/alarm : for OpenHAB; 1 if local alarm was triggered; 0 if local alarm was cancelled (whether from pressing the button, or if water leak is gone)

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
const int R_min = 200;
const int R_max = 500; 

const unsigned long int DT_QUIET = 60000;  // Quiet time after pressing the button, ms
const unsigned long int DT_DEBOUNCE = 100; // Physical switch debounce time in ms

const int N_BEEPS = 2; // Number of short beeps in the alarm sound
const unsigned long int DT1_BUZZER = 250; // Short beeps duration, ms
const unsigned long int DT2_BUZZER = 2000 + 2*N_BEEPS*DT1_BUZZER; // Interval between groups of short beeps, ms
const unsigned long int DT1_BAD_SENSOR = 50; // Short beep / LED flush duration if bad (shorted) water sensor, ms
const unsigned long int DT2_BAD_SENSOR = 10000; // Interval between short beep / LED flushs if bad (shorted) water sensor, ms

const unsigned long int DT1_RED_LED = 200; // Short flushes duration, ms, for external alarm
const unsigned long int DT2_RED_LED = 2000; // Interval between groups of short flushes, ms
const unsigned long int DT3_RED_LED = 500; // Long flashes duration, ms, for local alarm

const unsigned long int DT_GREEN_LED = 500; // Green LED blinking period, when WiFi is on but MQTT is not connected

const unsigned long int DT_WATER = 10; // Interval between water sensor readings, ms (should be at least 10ms, or it will affect WiFi connection)

#ifdef DEBUG
const unsigned long int DT_PRINT = 500;  // interval between water sensor prints, ms
#endif

// If required, each sensor can have some parameters customized here
// The same approach can be used inside private.h (e.g., to customize ssid and/or wifi passwords, repending on the sensor location)
#if SENSOR_ID == 1
// Green LED brightness (0-255):
const byte LED_PWM = 10;

#elif SENSOR_ID == 2
// Green LED brightness (0-255):
const byte LED_PWM = 10;
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
byte quiet; // =1 during quiet time, 0 otherwise
char buf[20];
#ifdef DEBUG
unsigned long int t_print;
#endif
