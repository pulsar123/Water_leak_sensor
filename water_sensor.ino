void water_sensor()
// Reading the water (resistance) sensor
{

  // This is to have the sensor reading done once every DT_WATER ms
  // Without this, WiFi may not work (likely due to EM intereference)
  if (millis() - t_water < DT_WATER)
    return;

  t_water = millis();

  // Reading the water sensor
  // Assumes that the analog pin is pulled down with ~200k resistor
  // Small R values correspond to small resistance
  int R = 1023 - analogRead(A0);

  byte sensor_state_temp;
  if (R >= R_min && R <= R_max)
    // Water detection
    sensor_state_temp = 1;
  else if (R < R_min)
    // Bad (shorted) sensor
    sensor_state_temp = 2;
  else
    // No issues
    sensor_state_temp = 0;

#ifdef DEBUG
#ifdef PRINT_SENSOR
  if (millis() - t_print > DT_PRINT)
  {
    Serial.print("R=");
    Serial.println(R);
    t_print = millis();
  }
#endif
#endif

// "Debouncing" for the water sensor: no more than one state change in DT_WATER_DEBOUNCE ms
// (Reduces the measurement "noise", when alarm would go on and off many times per second)
  if ((t_sensor == 0) || (sensor_state_temp != sensor_state) && (millis() - t_sensor > DT_WATER_DEBOUNCE))
  {
    
    // Not detecting water for DT_QUIET after the button was pressed:
    if (!quiet)
      if (local_alarm == 0 && sensor_state_temp == 1)
        // Water has just been detected
      {
        local_alarm = 1;
        external_alarm = 0; // Local alarm overrides an external one
        t_alarm = millis();
        t_buzzer = t_alarm;
        t_red_led = t_alarm;
        red_led_flag = 0;
        buz_flag = 0;
        if (MQTT_on)
        {
          // Notifying other water sensors (including the local sensor ID as the value):
          sprintf(buf, "%d", SENSOR_ID);
          client.publish(ROOT"/alarm", buf);
          // Notifying OpenHAB:
          client.publish(ROOT_ID"/alarm", "1");
        }
#ifdef DEBUG
        Serial.println("local_alarm=1");
#endif
      }

    if (local_alarm == 1 && (sensor_state_temp==0 || sensor_state_temp==2))
      // No more water; calling off the alarm
    {
      local_alarm = 0;
      t_alarm = 0;
      if (MQTT_on)
      {
        client.publish(ROOT"/alarm", "0");
        client.publish(ROOT_ID"/alarm", "0");
      }
#ifdef DEBUG
      Serial.println("local_alarm=0");
#endif
    }

    if (bad_sensor==0 && sensor_state_temp==2)
      // Too small resistance - likely shorted water sensor
    {
      bad_sensor = 1;
      bad_sensor_flag = 0;
      t_bad_sensor = millis();
    }

    if (bad_sensor==1 && sensor_state_temp!=2)
      // Water sensor is no longer bad
    {
      bad_sensor = 0;
      bad_sensor_flag = 0;
      t_bad_sensor = 0;
    }

    sensor_state = sensor_state_temp;
    t_sensor = millis();
  }
  
  return;
}

