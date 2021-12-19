#ifndef _wifi_configuration_
#define _wifi_configuration_

#include <ESP8266WiFi.h>

// WiFi access point
//#define WIFI_AP_NAME "aula_FLS_SPh"
#define WIFI_AP_NAME "aula8_FLS_tplink"
// #define WIFI_AP_NAME "TL4k HUAWEI P20"
// WiFi password
#define WIFI_PASSWORD "fablabsulcis"
// #define WIFI_PASSWORD "password"

// the Wifi radio's status
int status = WL_IDLE_STATUS;

// Initialize ThingsBoard client
WiFiClient espClient;

void InitWiFi()
{
  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  Serial.println("Trying to Connect to AP");
  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");

}

void reconnectWifi()
{
  // Loop until we're reconnected
  status = WiFi.status();
  Serial.println("Trying to Re-connect to AP");
  if (status != WL_CONNECTED)
  {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }

  // connectThingsBoard();
}

void checkWifiConnectionAndRecconnectIfNecessary()
{
  // Reconnect to WiFi, if needed
  bool isWifiConnected = WiFi.status() != WL_CONNECTED;
  if (isWifiConnected)
  {
    Serial.println("Connecting to: WIFI");
    reconnectWifi();
    return;
  }
}
#endif //_wifi_configuration_