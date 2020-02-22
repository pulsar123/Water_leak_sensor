void led ()
// Operating the LEDs.
{
  if (external_alarm)
  // Pattern blinking if external alarm is received (number of short flushes = ID of the alarming sensor)
  {
    unsigned long int dt = millis() - t_red_led;

//  Creating a gap of DT2_RED_FLAG ms between series of flashes:
    if (dt >= DT2_RED_LED + 2*id_alarm*DT1_RED_LED && red_led_flag==2*(int)id_alarm)
    // Resetting the LED sequence DT2_RED_LED ms after the flashing sequence
    {
      dt = 0;
      t_red_led = millis();
      red_led_flag = 0;
    }

    red_led = 1;
    for (int i=0; i < 2*(int)id_alarm; i++)
    {
      if (dt >= i*DT1_RED_LED && red_led_flag==i)
      {
        // Turning the red LED on or off:
        digitalWrite(RED_LED_PIN, red_led); // even/odd i -> 1/0
        red_led_flag = i+1;
        break;
      }
      red_led = 1 - red_led;
    }
  }

  if (local_alarm)
  // Constant blinking if local alarm goes off
  {
    unsigned long int dt = millis() - t_red_led;

    if (red_led_flag == 0)
    {
      red_led = 1;
      digitalWrite(RED_LED_PIN, red_led);
      red_led_flag = 1;
    }

    if (dt >= DT3_RED_LED && red_led_flag==1)
    {
      red_led = 0;
      digitalWrite(RED_LED_PIN, red_led);
      red_led_flag = 2;      
      t_red_led = millis();
      dt = 0;
    }

    if (dt >= DT3_RED_LED && red_led_flag==2)
    {
      red_led = 1;
      digitalWrite(RED_LED_PIN, red_led);
      red_led_flag = 1;      
      t_red_led = millis();
    }
  }

  if (bad_sensor)
  // Short LED flushes if bad (shorted) water sensor
  {
    unsigned long int dt = millis() - t_bad_sensor;
    // Start of the initial flash:
    if (bad_sensor_flag==0 && dt < DT1_BAD_SENSOR)
    {
      bad_sensor_flag = 1;
      red_led = 1;
      digitalWrite(RED_LED_PIN, red_led);      
    }

    // End of a short flash:
    if (bad_sensor_flag==1 && dt >= DT1_BAD_SENSOR)
    {
      bad_sensor_flag = 2;
      t_bad_sensor = millis();
      red_led = 0;
      digitalWrite(RED_LED_PIN, red_led);      
    }

    // Start of a short flash:
    if (bad_sensor_flag==2 && dt >= DT2_BAD_SENSOR)
    {
      bad_sensor_flag = 1;
      t_bad_sensor = millis();
      red_led = 1;
      digitalWrite(RED_LED_PIN, red_led);      
    }
}

// Cleaning up:
  if (external_alarm==0 && local_alarm==0 && bad_sensor==0 && red_led==1)
  {
    red_led = 0;
    digitalWrite(RED_LED_PIN, red_led);
  }


//----------------------------------------------------------------------------
// Green LED stuff

// Green LED is blinking when WiFi is on but MQTT is not connected:
  if (WiFi_on==1 && MQTT_on==0 && millis()-t_green_led > DT_GREEN_LED)
  {
    t_green_led = millis();
    // Flipping the state of the green LED every DT_GREEN_LED ms:
    green_led = LED_PWM - green_led;
    analogWrite(GREEN_LED_PIN, green_led);
  }

  // Cleanup:
  if (MQTT_on==1 && green_led==0)
  {
    // Making sure the green LED is on once the MQTT connection is established (which can only happen if WiFi is also on):
    green_led = LED_PWM;
    analogWrite(GREEN_LED_PIN, green_led);    
  }
  
  return;
}

