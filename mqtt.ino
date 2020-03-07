void mqtt()
// Reading / sending MQTT message(s)
{
  if (MQTT_on)
  {
    // Receiving; calls callback() function internally:
    client.loop();

    // Re-publishing to OpenHAB
    if (mqtt_init || mqtt_refresh)
    {
      if (local_alarm)
        client.publish(ROOT_ID"/alarm", "1");
      else
        client.publish(ROOT_ID"/alarm", "0");
      client.publish(ROOT_ID"/pulse", "1");
      t_pulse = millis();
    }

    mqtt_init = 0;
    mqtt_refresh = 0;

    // "Pulse" functionality: publishing this every DT_PULSE to let OpenHAB know that the sensor is alive
    if (t_pulse == 0 || millis() - t_pulse > DT_PULSE)
    {
      client.publish(ROOT_ID"/pulse", "1");
      t_pulse = millis();
    }

  }


  return;
}
