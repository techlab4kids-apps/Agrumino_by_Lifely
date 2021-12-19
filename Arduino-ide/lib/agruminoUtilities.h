#ifndef _Agrumino_Utilities_h_
#define _Agrumino_Utilities_h_
#include "Agrumino.h"

typedef struct AgruminoData_t
{
  float temperature;
  unsigned int soilMoisture;
  float illuminance;
  float batteryVoltage;
  unsigned int batteryLevel;
  boolean isAttachedToUSB;
  boolean isBatteryCharging;
  boolean isLedOn;
  String ledColor;
} AgruminoData;

#define TEMPERATURE_CORRECTION 9

#ifndef COUNT_OF
// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#endif

Agrumino agrumino;

/////////////////////
// Utility methods //
/////////////////////

void toggleLed()
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

void setLedStatus(bool status){
  if(status){
    agrumino.turnLedOn();
  }
  else{
    agrumino.turnLedOff();
  }
}

void blinkLed(int duration, int blinks)
{
  Serial.println("Start led blinking");
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
  Serial.println("Led blinking ended");
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
  agruminoData.isLedOn = agrumino.isLedOn();
  agruminoData.ledColor = agrumino.isLedOn() ? "\"#54C0DA\"" : "\"#022B59\"";
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
  Serial.println("isLedOn: " + String(agruminoData.isLedOn));
  Serial.println();
}

#endif //_Agrumino_Utilities_h_
