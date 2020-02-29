void read_switch()
// Reading the switch state
{

  // 0: off, 1: on (assuming the pin is pulled up)
  byte switch_state_temp = 1 - digitalRead(SWITCH_PIN);

  // Using debouncing to prevent multiple false "turn on" states within a short interval (DT_DEBOUNCE ms) due to switch noise:
  if (switch_state_temp != switch_state && (t_switch == 0 || millis() - t_switch > DT_DEBOUNCE))
  {
    if (switch_state == 0 && switch_state_temp == 1)
      // Switch was just turned on
    {
      if (local_alarm || external_alarm)
        // Disabling the alarm, for DT_QUIET ms
      {
        // Only local alarm can be silenced for some time:        
        if (local_alarm)
        {
          t_quiet = millis();
          quiet = 1;
        }
//??        local_alarm = 0;
        // External alarm is silenced, but would go off at any time later if/when triggered
        external_alarm = 0;
        // Sending MQTT quiet command to other sensors
        if (MQTT_on)
          client.publish(ROOT"/quiet", "0");
      }
#ifdef BUZZER_TEST
      buzzer_on = 1 - buzzer_on;
#endif
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


  return;
}


