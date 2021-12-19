#include <Arduino.h>


#include "lib/wifiConfiguration.h"
#include "lib/agruminoUtilities.h"
#include "lib/mqttBrokerConfiguration.h"


// Agrumino information

// Time to sleep in second between the readings/data sending
#define SLEEP_TIME 300       // milliseconds
#define SEND_DATA_DELAY 30000 // milliseconds

int lastSend;


void setup()
{
  // put your setup code here, to run once:

  // initialize serial port
  Serial.begin(115200);

  // Setup our super cool lib
  agrumino.setup();

  // Turn on the board to allow the usage of the Led
  agrumino.turnBoardOn();
  blinkLed(500, 3);

  InitWiFi();
  
  connectToMqttBroker();
  
  // connectThingsBoard();

}

void loop()
{
  // put your main code here, to run repeatedly:

  //  agrumino.turnBoardOn();

  checkWifiConnectionAndRecconnectIfNecessary();

  // checkTbConnectionAndReconnectIfRequired();
  
  checkMqttBrokerConnectionAndReconnectIfRequired();

  // Check if it is a time to send agrumino sensors data
  bool isTimeToSendData = millis() - lastSend > SEND_DATA_DELAY;
  if (isTimeToSendData)
  {
    AgruminoData agruminoData;

    Serial.println("#########################\n");

    // loop main code
    getAgruminoData(agruminoData);

    printAgruminoData(agruminoData);

    sendAgruminoDataToMqttBroker(agruminoData);

    //sendAgruminoDataToTb(agruminoData);

    // END loop main code

    lastSend = millis();
  }

  // Process messages
  
  //tb.loop();
  
  mqttBrokerLoop();

  // time to sleep!
  delay(SLEEP_TIME);
}