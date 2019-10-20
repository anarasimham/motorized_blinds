// **********************************************************************************************************
// Moteino gateway/base sketch that works with Moteinos equipped with RFM69W/RFM69HW/RFM69CW/RFM69HCW
// This is a basic gateway sketch that receives packets from end node Moteinos, formats them as ASCII strings
//      with the end node [ID] and passes them to Pi/host computer via serial port
//     (ex: "messageFromNode" from node 123 gets passed to serial as "[123] messageFromNode")
// It also listens to serial messages that should be sent to listening end nodes
//     (ex: "123:messageToNode" sends "messageToNode" to node 123)
// Make sure to adjust the settings to match your transceiver settings (frequency, HW etc).
// **********************************************************************************
// Copyright Felix Rusu 2016, http://www.LowPowerLab.com/contact
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#include <RFM69.h>         //get it here: https://github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://github.com/lowpowerlab/RFM69
#include <RFM69_OTA.h>     //get it here: https://github.com/lowpowerlab/RFM69
//#include <SPIFlash.h>      //get it here: https://github.com/lowpowerlab/spiflash
#include <SPI.h>           //included with Arduino IDE (www.arduino.cc)

#include <TimeLib.h>
#include <stdio.h>

#define TIME_HEADER  "TI"   // Header tag for serial time sync message
#define TIME_REQUEST  "SYN-REQ"    // ASCII bell character requests a time sync message 
//****************************************************************************************************************
//**** IMPORTANT RADIO SETTINGS - YOU MUST CHANGE/CONFIGURE TO MATCH YOUR HARDWARE TRANSCEIVER CONFIGURATION! ****
//****************************************************************************************************************
#define NODEID          1 //the ID of this node
#define NETWORKID     200 //the network ID of all nodes this node listens/talks to
#define FREQUENCY     RF69_915MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define FREQUENCY_EXACT 916000000
#define ENCRYPTKEY    "sampleEncryptKey" //identical 16 characters/bytes on all nodes, not more not less!
//*****************************************************************************************************************************
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI      -75  //target RSSI for RFM69_ATC (recommended > -80)
//*****************************************************************************************************************************
// Serial baud rate must match your Pi/host computer serial port baud rate!
#define SERIAL_EN     //comment out if you don't want any serial verbose output
#define SERIAL_BAUD  115200
//*****************************************************************************************************************************

#define TIMEOUT     3000
#define ACK_TIME       80  // # of ms to wait for an ack packet
#define RETRIES       200

#define DEBUG_MODE false         //set 'true' to see verbose output from programming sequence

#ifdef __AVR_ATmega1284P__
#define LED           15 // Moteino MEGAs have LEDs on D15
#define FLASH_SS      23 // and FLASH SS on D23
#else
#define LED           9 // Moteinos have LEDs on D9
#define FLASH_SS      8 // and FLASH SS on D8
#endif

#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif

#ifdef ENABLE_ATC
RFM69_ATC radio;
#else
RFM69 radio;
#endif

//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit Windbond FlashMEM chip

enum blindsStates {
  BLINDS_OPEN,
  BLINDS_CLOSED,
  BLINDS_CUSTOM
};
enum blindsStates blindsState;
boolean blindsValidState = true;

int nodeList[1];

char input[64];
byte buff[61];
String inputstr;
byte targetID = 0;

void setup() {
  blindsState = BLINDS_CLOSED;
  nodeList[0] = 2;
  
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.encrypt(ENCRYPTKEY);

#ifdef FREQUENCY_EXACT
  radio.setFrequency(FREQUENCY_EXACT); //set frequency to some custom frequency
#endif


#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", radio.getFrequency() / 1000000);
  DEBUGln(buff);


  /*
    if (flash.initialize())
    {
      DEBUGln("SPI Flash Init OK!");
    }
    else
    {
      DEBUGln("SPI FlashMEM not found (is chip onboard?)");
    }
  */

  setSyncProvider( requestSync);  //set function to call when sync required
}

void loop() {
  byte inputLen = readSerialLine(input, 10, 64, 100);

  if (inputLen == 4 && input[0] == 'F' && input[1] == 'L' && input[2] == 'X' && input[3] == '?') {
    if (targetID == 0)
      Serial.println("TO?");
    else
    {
      char payload[] = "LSN";
      Serial.println("Asking node to listen");
      if (radio.sendWithRetry(nodeList[0], payload, strlen(payload), 250, 200)) {
        Serial.println("Told node to listen");
        CheckForSerialHEX((byte*)input, inputLen, radio, targetID, TIMEOUT, ACK_TIME, DEBUG_MODE);
      }
    }
  }
  else if (inputLen > 3 && inputLen <= 6 && input[0] == 'T' && input[1] == 'O' && input[2] == ':')
  {
    byte newTarget = 0;
    for (byte i = 3; i < inputLen; i++) //up to 3 characters for target ID
      if (input[i] >= 48 && input[i] <= 57)
        newTarget = newTarget * 10 + input[i] - 48;
      else
      {
        newTarget = 0;
        break;
      }
    if (newTarget > 0)
    {
      targetID = newTarget;
      Serial.print("TO:");
      Serial.print(newTarget);
      Serial.println(":OK");
    }
    else
    {
      Serial.print(input);
      Serial.print(":INV");
    }
  }
  else if (inputLen > 0) { //just echo back
    Serial.print("SERIAL IN > "); Serial.println(input);
  }

  if (radio.receiveDone())
  {
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);

    if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }

    Serial.println();
    Blink(LED, 3);
  }

  if (inputLen > 0) {
    if (input[0] == 'B') {
      if (input[1] == 'C') {
        blindsState = BLINDS_CLOSED;
        blindsValidState = false;
      } else if (input[1] == 'O') {
        blindsState = BLINDS_OPEN;
        blindsValidState = false;
      } else {
        blindsState = BLINDS_CUSTOM;
        blindsValidState = false;
      }
    }

    inputstr = String(input);
    processSyncMessage(inputstr);
  }
  if (hour() == 9 && minute() == 00 && second() < 20) {
    blindsState = BLINDS_OPEN;
    blindsValidState = false;
  }

  if (hour() == 20 && minute() == 0 && second() < 20) {
    blindsState = BLINDS_CLOSED;
    blindsValidState = false;
  }



  if (!blindsValidState) {
    if (blindsState == BLINDS_OPEN) {
      openBlinds();
    } else if (blindsState == BLINDS_CLOSED) {
      closeBlinds();
    } else {
      char msg[5];
      msg[4] = '\0';
      for (byte i = 0; i < 4; i++) {
        msg[i] = input[i];
      }
        DEBUG("'");DEBUG(msg);DEBUG("'");DEBUGln();
      sendBlindsMessage(msg);
    }
    blindsValidState = true;
  }

  //delay(10000);

}

void openBlinds() {
  char payload[] = "BO";
  sendBlindsMessage(payload);
}

void closeBlinds() {
  char payload[] = "BC";
  sendBlindsMessage(payload);
}

void sendBlindsMessage(char* payload) {
  char buf[50];
  for (byte i = 0; i < sizeof(nodeList) / sizeof(int); i++) {
    if (!radio.sendWithRetry(nodeList[i], payload, strlen(payload), RETRIES, ACK_TIME)) {

      sprintf(buf, "Failed sending payload %s to node %d, no ACK", payload, nodeList[i]);
      Serial.println(buf);
    } else {
      sprintf(buf, "Sent payload %s to node %d, RSSI: %d", payload, nodeList[i], radio.RSSI);
      Serial.println(buf);
    }
  }
}

void processSyncMessage(String inp) {
  signed long pctime;
  char offsetSign;
  int signLoc;
  signed int offsetNum;
  String fullTime;
  const signed long DEFAULT_TIME = 1357041600; // Jan 1 2013


  if (inp.substring(0, 2) == TIME_HEADER) {
    DEBUGln("Syncing time '" + inp + "'");
    fullTime = inp.substring(2);
    signLoc = fullTime.indexOf('-');
    if (signLoc < 0) {
      signLoc = fullTime.indexOf('+');
    }

    pctime = fullTime.substring(0, signLoc).toInt();
    offsetSign = fullTime.charAt(signLoc);
    offsetNum = fullTime.substring(signLoc + 1).toInt();
    if (offsetSign == '-') {
      offsetNum *= -1;
    }
    pctime = pctime + offsetNum * 60 * 60;
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }
  }
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}

/*
  void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
  }

  void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
  }
*/

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
