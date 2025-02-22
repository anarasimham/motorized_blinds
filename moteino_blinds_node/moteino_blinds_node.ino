// Sample RFM69 sender/node sketch, with ACK and optional encryption, and Automatic Transmission Control
// Sends periodic messages of increasing length to gateway (id=1)
// It also looks for an onboard FLASH chip, if present
// **********************************************************************************
// Copyright Felix Rusu 2018, http://www.LowPowerLab.com/contact
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
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_OTA.h>
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash
#include <avr/wdt.h>

#include <Servo.h>
#include <LowPower.h>
//*********************************************************************************************
//************ IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE ************
//*********************************************************************************************
// Address IDs are 10bit, meaning usable ID range is 1..1023
// Address 0 is special (broadcast), messages to address 0 are received by all *listening* nodes (ie. active RX mode)
// Gateway ID should be kept at ID=1 for simplicity, although this is not a hard constraint
//*********************************************************************************************
#define NODEID        2    // keep UNIQUE for each node on same network
#define NETWORKID     200  // keep IDENTICAL on all nodes that talk to each other

//*********************************************************************************************
// Frequency should be set to match the radio module hardware tuned frequency,
// otherwise if say a "433mhz" module is set to work at 915, it will work but very badly.
// Moteinos and RF modules from LowPowerLab are marked with a colored dot to help identify their tuned frequency band,
// see this link for details: https://lowpowerlab.com/guide/moteino/transceivers/
// The below examples are predefined "center" frequencies for the radio's tuned "ISM frequency band".
// You can always set the frequency anywhere in the "frequency band", ex. the 915mhz ISM band is 902..928mhz.
//*********************************************************************************************
//#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define FREQUENCY_EXACT 916000000 // you may define an exact frequency/channel in Hz
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
//*********************************************************************************************
//Auto Transmission Control - dials down transmit power to save battery
//Usually you do not need to always transmit at max output power
//By reducing TX power even a little you save a significant amount of battery power
//This setting enables this gateway to work with remote nodes that have ATC enabled to
//dial their power down to only the required level (ATC_RSSI)
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI      -75
//*********************************************************************************************
#define SERIAL_BAUD   115200
#define MOTORCONTROLPIN 10
#define LISTEN_PERIOD 7
//**** SETTINGS *********************************************************************************************
#define WITH_RFM69              //comment this line out if you don't have a RFM69 on your Moteino
#define WITH_SPIFLASH           //comment this line out if you don't have the FLASH-MEM chip on your Moteino
//***********************************************************************************************************

#if defined(WITH_RFM69) || defined(WITH_SPIFLASH)
  #include <SPI.h>                //comes with Arduino IDE (www.arduino.cc)
  #if defined(WITH_RFM69)
    #include <RFM69.h>            //get it here: https://www.github.com/lowpowerlab/rfm69
    #ifdef ENABLE_ATC
      RFM69_ATC radio;
    #else
      RFM69 radio;
    #endif
  #endif
  #if defined(WITH_SPIFLASH)
    #include <SPIFlash.h>         //get it here: https://www.github.com/lowpowerlab/spiflash
    SPIFlash flash(SS_FLASHMEM, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
  #endif
#endif


#if defined (MOTEINO_M0) && defined(SERIAL_PORT_USBVIRTUAL)
  #define Serial SERIAL_PORT_USBVIRTUAL // Required for Serial on Zero based boards
#endif

Servo myServo;
int servoDegs;

void setup() {
#ifdef WITH_RFM69
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.sleep();
#endif

#ifdef WITH_SPIFLASH
  if (flash.initialize())
    flash.sleep();
#endif

#ifdef ENCRYPTKEY
  radio.encrypt(ENCRYPTKEY);
#endif

#ifdef FREQUENCY_EXACT
  radio.setFrequency(FREQUENCY_EXACT); //set frequency to some custom frequency
#endif
  
//Auto Transmission Control - dials down transmit power to save battery (-100 is the noise floor, -90 is still pretty good)
//For indoor nodes that are pretty static and at pretty stable temperatures (like a MotionMote) -90dBm is quite safe
//For more variable nodes that can expect to move or experience larger temp drifts a lower margin like -70 to -80 would probably be better
//Always test your ATC mote in the edge cases in your own environment to ensure ATC will perform as you expect
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

#ifdef ENABLE_ATC
  Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
#endif

  servoDegs = -1;

  for (uint8_t i=0; i<=A5; i++)
  {
#ifdef WITH_RFM69
    if (i == RF69_SPI_CS) continue;
#endif
#ifdef WITH_SPIFLASH
    if (i == SS_FLASHMEM) continue;
#endif
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  pinMode(0, OUTPUT);
  digitalWrite(0,HIGH);
  pinMode(1, OUTPUT);
  digitalWrite(1,HIGH);

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(20);
  digitalWrite(LED_BUILTIN, LOW);
  delay(20);

  Serial.begin(SERIAL_BAUD);
  Serial.println("Done setting up");
  Serial.flush();
}

/// Use watchdog to reset
void resetUsingWatchdog(boolean DEBUG)
{
  //wdt_disable();
  if (DEBUG) Serial.print("REBOOTING");
  wdt_enable(WDTO_15MS);
  while(1) if (DEBUG) Serial.print('.');
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

void sweep(int toDeg) {
  myServo.attach(MOTORCONTROLPIN);
  
  if (servoDegs == -1) {
    myServo.write(toDeg);
    servoDegs = toDeg;
  } else {
    while (servoDegs != toDeg) {
      if (servoDegs < toDeg) {
        servoDegs += 1;
      } else {
        servoDegs -= 1;
      }

      myServo.write(servoDegs);
      delay(15);
    }
  }
}

void loop() {
  //check for any received packets
  radio.receiveDone();
  delay(LISTEN_PERIOD);
  
  if (radio.receiveDone())
  {
    
    byte datalen = radio.DATALEN;
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    //If new code received, download new code and reboot
    CheckForWirelessHEX(radio, flash, true);
    
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    //Blink(LED_BUILTIN,3);
    Serial.println();
    
    
    if (datalen == 3 
      && radio.DATA[0] == 'L' 
      && radio.DATA[1] == 'S'
      && radio.DATA[2] == 'N') {
      resetUsingWatchdog(false);
    }

    if (datalen == 2 
      && radio.DATA[0] == 'B' 
        && ( radio.DATA[1] == 'O' 
        ||   radio.DATA[1] == 'C') ) {
      
      if (radio.DATA[1] == 'O') {
        Serial.println("Got BO");
        sweep(90);
      } else {
        Serial.println("Got BC");
        sweep(180);
      }
    }

    if ((datalen == 4)
      && radio.DATA[0] == 'B') {
      String degStr = "";
      for (byte i = 1; i < 4; i++) {
        degStr += (char)radio.DATA[i];
      }
      
      int degs = degStr.toInt();
      sweep(degs);
    }
    delay(2000);
  }

  Serial.println("Powering down 8s");
  Serial.flush();
  radio.sleep();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
