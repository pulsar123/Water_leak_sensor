void read_switch()
{

  // 0: off, 1: on (assuming the pin is pulled up)
  byte switch_state_temp = 1 - digitalRead(SWITCH_PIN);

  if (switch_state_temp != switch_state && (t_switch==0 || millis() - t_switch > DT_DEBOUNCE))
  {
    if (switch_state == 0 && switch_state_temp == 1)
      // Switch was just turned on
    {
      if (local_alarm || external_alarm)
      // Disabling the alarm, for DT_QUIET ms
      {
        local_alarm = 0;
        external_alarm = 0;
        t_quiet = millis();
        // Sending MQTT quiet command to other sensors
        if (MQTT_on)
          client.publish(ROOT"/alarm", "0");
      }
    }

/*
    if (switch_state == 1 && switch_state_temp == 0)
      // Switch was just turned off
    {
    }
*/
    
    switch_state = switch_state_temp;
    t_switch = millis();
  }

// This is to prevent t_quiet to be overfilled
// Beware the t_quiet can be set either in this function, or in callback() (from external sensors)
  if (switch_state==0 && millis() - t_quiet > DT_QUIET_RESET)
  {
    t_quiet = 0;
  }

  return;
}


