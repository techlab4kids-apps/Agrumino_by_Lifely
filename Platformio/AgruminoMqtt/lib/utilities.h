#ifndef Agrumino_Utilities_h
#define Agrumino_Utilities_h
#include "Agrumino.h"
#endif

#define TEMPERATURE_CORRECTION 5

#ifndef COUNT_OF
// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#endif

extern Agrumino agrumino;

/////////////////////
// Utility methods //
/////////////////////

void switchLed()
{
  if (agrumino.isLedOn())
  {
    Serial.println("Switching led OFF");
    agrumino.turnLedOff();
  }
  else
  {
    Serial.println("Switching led ON");
    agrumino.turnLedOn();
  }
}

void blinkLed(int duration, int blinks)
{
  for (int i = 0; i < blinks; i++)
  {
    agrumino.turnLedOn();
    delay(duration);
    agrumino.turnLedOff();
    if (i < blinks)
    {
      delay(duration); // Avoid delay in the latest loop ;)
    }
  }
}

void delaySec(int sec)
{
  delay(sec * 1000);
}

void deepSleepSec(int sec)
{
  Serial.print("\nGoing to deepSleep for ");
  Serial.print(sec);
  Serial.println(" seconds... (ー。ー) zzz\n");
  ESP.deepSleep(sec * 1000000); // microseconds
}

const String getChipId()
{
  // Returns the ESP Chip ID, Typical 7 digits
  return String(ESP.getChipId());
}

float agruminoGetTemperature()
{
  float temperature = agrumino.readTempC();
  boolean isAttachedToUSB = agrumino.isAttachedToUSB();
  if (isAttachedToUSB)
  {
    return temperature - TEMPERATURE_CORRECTION;
  }
  else
  {
    return temperature;
  }
}

// If the Agrumino S1 button is pressed for 5 seconds then reset the wifi saved settings.
boolean checkIfResetWiFiSettings()
{

  int remainingsLoops = (5 * 1000) / 100;

  Serial.print("Check if reset WiFi settings: ");

  while (agrumino.isButtonPressed() && remainingsLoops > 0)
  {
    delay(100);
    remainingsLoops--;
    Serial.print(".");
  }

  if (remainingsLoops == 0)
  {
    // Reset Wifi Settings
    Serial.println(" YES!");
    return true;
  }
  else
  {
    Serial.println(" NO");
    return false;
  }
}

// Agrumino utilities
void getAgruminoData(AgruminoData &agruminoData)
{
  Serial.println("Reading data from sensors...");
  agruminoData.temperature = agruminoGetTemperature();
  agruminoData.soilMoisture = agrumino.readSoil();
  agruminoData.illuminance = agrumino.readLux();
  agruminoData.batteryVoltage = agrumino.readBatteryVoltage();
  agruminoData.batteryLevel = agrumino.readBatteryLevel();
  agruminoData.isAttachedToUSB = agrumino.isAttachedToUSB();
  agruminoData.isBatteryCharging = agrumino.isBatteryCharging();
}

void printAgruminoData(AgruminoData &agruminoData)
{
  Serial.println("Data read from sensors...");
  Serial.println("temperature:       " + String(agruminoData.temperature) + "°C");
  Serial.println("soilMoisture:      " + String(agruminoData.soilMoisture) + "%");
  Serial.println("illuminance :      " + String(agruminoData.illuminance) + " lux");
  Serial.println("batteryVoltage :   " + String(agruminoData.batteryVoltage) + " V");
  Serial.println("batteryLevel :     " + String(agruminoData.batteryLevel) + "%");
  Serial.println("isAttachedToUSB:   " + String(agruminoData.isAttachedToUSB));
  Serial.println("isBatteryCharging: " + String(agruminoData.isBatteryCharging));
  Serial.println();
}

void publishMqttMessage(AgruminoData &agruminoData)
{
  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += agruminoData.temperature; payload += ",";
  payload += "\"soilMoisture\":"; payload += agruminoData.soilMoisture; payload += ",";
  payload += "\"illuminance\":"; payload += agruminoData.illuminance; payload += ",";
  payload += "\"batteryVoltage\":"; payload += agruminoData.batteryVoltage; payload += ",";
  payload += "\"batteryLevel\":"; payload += agruminoData.batteryLevel; payload += ",";
  payload += "\"isAttachedToUSB\":"; payload += agruminoData.isAttachedToUSB; payload += ",";
  payload += "\"isBatteryCharging\":"; payload += agruminoData.isBatteryCharging; payload += ",";

  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );

  Serial.println("Sending data to MqttBbroker...");
  String clientName = getDeviceMqttClientName();
  mqttClient.publish( "v1/devices/" + clientName +"/telemetry", attributes );
}

void sendAgruminoDataToTb(AgruminoData &agruminoData)
{
  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  Serial.println("Sending data to Tb...");
  tb.sendTelemetryFloat("temperature", agruminoData.temperature);
  tb.sendTelemetryInt("soilMoisture", agruminoData.soilMoisture);
  tb.sendTelemetryFloat("illuminance", agruminoData.illuminance);
  tb.sendTelemetryFloat("batteryVoltage", agruminoData.batteryVoltage);
  tb.sendTelemetryInt("batteryLevel", agruminoData.batteryLevel);
  tb.sendTelemetryBool("isAttachedToUSB", agruminoData.isAttachedToUSB);
  tb.sendTelemetryBool("isBatteryCharging", agruminoData.isBatteryCharging);

  publishMqttMessage(agruminoData);

}


