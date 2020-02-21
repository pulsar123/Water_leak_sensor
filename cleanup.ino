void cleanup()
// End of the main loop cleanup
{

  switch_state_old = switch_state;
  bad_sensor_flag_old = bad_sensor_flag;
  #ifdef BUZZER_TEST
    buzzer_on_old = buzzer_on;
  #endif

  return;
}
