void cleanup()
// End of the main loop cleanup
{
  
// Ending the quiet period after DT_QUIET ms:
  if (quiet && millis() > t_quiet + DT_QUIET)
    quiet = 0;
  
  switch_state_old = switch_state;
  bad_sensor_flag_old = bad_sensor_flag;
  
  #ifdef BUZZER_TEST
    buzzer_on_old = buzzer_on;
  #endif

  return;
}
