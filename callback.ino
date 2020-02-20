void callback(char* topic, byte* payload, unsigned int length)
// The function processing incoming MQTT commands.
// The command is in payload[0..length-1] char array.
{

  if (strcmp(topic, ROOT"/alarm") == 0)
  {
    if (external_alarm == 1 && (char)payload[0] == '0')
      // External command to be quiet, for DT_QUIET ms
    {
      external_alarm = 0;
      t_quiet = millis();
#ifdef DEBUG
      Serial.println("external_alarm=0");
#endif
    }
    // No external alarm when the local alarm is on:
    else if (local_alarm == 0 && external_alarm == 0 && (char)payload[0] != '0')
      // External command to sound alarm
    {
      payload[length] = '\0'; // Make payload a string by NULL terminating it.
      id_alarm = atoi((char *)payload);
      // Just in case (to prevent reacting to its own alarm receieved as external one)
      if (id_alarm != SENSOR_ID)
      {
        external_alarm = 1;
        t_alarm = millis();
        t_buzzer = t_alarm;
        t_red_led = t_alarm;
        red_led_flag = 0;
        buz_flag = 0;
#ifdef DEBUG
        Serial.print("external_alarm=");
        Serial.println(id_alarm);
#endif
      }
    }
  }

  if (strcmp(topic, "openhab/start") == 0 && (char)payload[0] == '1')
    // We received a signal that openhab server has just restarted, so will re-publish all mqtt states later, in mqtt()
  {
    mqtt_refresh = 1;
  }

  return;
}

