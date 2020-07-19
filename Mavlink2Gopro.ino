
#include <ESP8266WiFi.h>
#include "GoProHero4.h"
#include "MavLinkDecoder.h"

//switch shutter position
bool shutterSwitch=false;

//remote control status
bool failSafe=true;



GoProHero4 *gopro;
MavLinkDecoder *mavLinkDecoder;


void setup() {

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  //Start communication to serial port
  Serial.begin(57600);
  
  //GOPRO MAC address used only for power on/
  //You can get it with http://10.5.5.9/gp/gpControl/info
  //and insert it below
  byte mac[] = { 0xD4, 0xD9, 0xFF, 0xFF, 0xFF, 0xFF };
  
  //Connect to your GoPro WIFI hotspot
  gopro=new GoProHero4("SSID", "Password",mac);

  //creation of Mavlink decoder
  mavLinkDecoder=new MavLinkDecoder();
}

void loop()
{  
  //process mavlink input stream
  mavLinkDecoder->comm_receive();
  
  if (failSafe && !mavLinkDecoder->failSafe()) {
    failSafe=false;
    Serial.println("Failsafe deactivated");
  }
  else if (!failSafe && mavLinkDecoder->failSafe()) {
    failSafe=true;
    Serial.println("Failsafe activated");
  }

   if (gopro->isConnected()) {
    
    //if Gopro is off, switch it On
    if (gopro->power==GoProHero4::Off)  gopro->powerOn();

    //else if failsave not active
    else if (!failSafe) {
      //mode selection with channel 8
      //below 0% Video Mode
      if ( (gopro->currentMode==GoProHero4::Photo) && mavLinkDecoder->channel8< (MavLinkDecoder::CTRL_CENTER-MavLinkDecoder::CTRL_DEAD_ZONE))  gopro->videoMode();
      //above 0% Photo Mode
      else if ( (gopro->currentMode==GoProHero4::Video) && mavLinkDecoder->channel8> (MavLinkDecoder::CTRL_CENTER+MavLinkDecoder::CTRL_DEAD_ZONE))  gopro->photoMode();
      
      //shutter triggering with channel 8 , with channel value between -50% and 0% or 50% and 100%
      if ( ! shutterSwitch &&
        ( ( (mavLinkDecoder->channel8> (MavLinkDecoder::CTRL_M50+MavLinkDecoder::CTRL_DEAD_ZONE)) && (mavLinkDecoder->channel8< (MavLinkDecoder::CTRL_CENTER-MavLinkDecoder::CTRL_DEAD_ZONE))) || 
          (mavLinkDecoder->channel8> (MavLinkDecoder::CTRL_P50+MavLinkDecoder::CTRL_DEAD_ZONE) ))
        ){
        shutterSwitch=true;
        gopro->photoCapture();
      }
      else if ( shutterSwitch && 
        ( (mavLinkDecoder->channel8< (MavLinkDecoder::CTRL_M50-MavLinkDecoder::CTRL_DEAD_ZONE))|| ( (mavLinkDecoder->channel8> (MavLinkDecoder::CTRL_CENTER+MavLinkDecoder::CTRL_DEAD_ZONE)) && (mavLinkDecoder->channel8< (MavLinkDecoder::CTRL_P50-MavLinkDecoder::CTRL_DEAD_ZONE)))) 
        
        ) shutterSwitch=false;
    }

   }
}
 
