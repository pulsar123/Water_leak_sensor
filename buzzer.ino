void buzzer ()
// Operating the buzzer during alarm (N_BEEPS short beeps, DT1_BUZZER ms each, repeated every DT2_BUZZER ms)
{

  if (external_alarm || local_alarm)
  {
    unsigned long int dt = millis() - t_buzzer;

    if (dt >= DT2_BUZZER && buz_flag==4)
    // Resetting the buzzer sequence every DT2_BUZZER ms
    {
      dt = 0;
      t_buzzer = millis();
      buz_flag = 0;
    }

    buzzer_state = 1;
    for (int i=0; i < 2*N_BEEPS; i++)
    {
      if (dt >= i*DT1_BUZZER && buz_flag==i)
      {
        // Turning the buzzer on or off:
#ifndef QUIET        
        digitalWrite(BUZZER_PIN, buzzer_state); // even/odd i -> 1/0
#endif        
        buz_flag = i+1;
        break;
      }
      buzzer_state = 1 - buzzer_state;
    }
  }

if (bad_sensor && bad_sensor_flag!=bad_sensor_flag_old)
  // Short beeps if bad (shorted) water sensor; using red LED timers / flags for control
  {
    // End of a short beep:
    if (bad_sensor_flag == 1)
    {
      buzzer_state = 0;
      digitalWrite(BUZZER_PIN, buzzer_state); 
    }

    // Start of a short beep:
    if (bad_sensor_flag == 2)
    {
      buzzer_state = 1;
      digitalWrite(BUZZER_PIN, buzzer_state); 
    }
}


// Cleaning up after an alarm:
  if (!(external_alarm || local_alarm || bad_sensor) && buzzer_state==1)
  {
    buzzer_state = 0;
#ifndef QUIET        
    digitalWrite(BUZZER_PIN, buzzer_state); 
#endif    
  }

  return;
}

