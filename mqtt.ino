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
    }

    mqtt_init = 0;
    mqtt_refresh = 0;
  }
}
