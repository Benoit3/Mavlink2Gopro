//Library for GOPRO Hero4 silver written with the help of:
//https://github.com/KonradIT/goprowifihack/blob/master/HERO4/WifiCommands.md

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include "GoProHero4.h"

//GOPRO MAC address used only for power on/
//You can get it with http://10.5.5.9/gp/gpControl/info
byte *GoProHero4::mac;

//GOPRO IP address. You normally never have to modify this line
const char* GoProHero4::host = "10.5.5.9"; //GoPro Hero 4 IP Address



GoProHero4::GoProHero4(char* ssid,char* passwd,byte* mac) {

      //save mac address pointer
      GoProHero4::mac=mac;
      
      //init link status
      link=Disconnected;
      
      //Wifi Connect
      Serial.print("Connecting to "); Serial.println(ssid);
      WiFi.begin(ssid,passwd);
}
    

//refresh status attributes of the GoPro
void GoProHero4::updateStatus(DynamicJsonDocument GoProStatus) {

  //if status array is present
  if (GoProStatus.containsKey("status")) {

    //update of current mode
    if (GoProStatus["status"].containsKey("43")){
      currentMode=(ModeValue)GoProStatus["status"]["43"].as<int>();
      Serial.print("Current Mode:"); Serial.println(currentMode);
    }
    //update Recording Processing Status
     if (GoProStatus["status"].containsKey("8")){
      recordingProcessing=(RecordingProcessingState)GoProStatus["status"]["8"].as<int>();
      Serial.print("Recording/Processing :"); Serial.println(recordingProcessing);
    }   
    else Serial.println("Current Mode not found in record status");
  } else Serial.println("Status record not found");
  
  if (GoProHero4::link==GoProHero4::Connected) {

    
  }
}


int GoProHero4::ExecGoProCmd(String GoProURL) {
      
      //if the WIFI connection to the GoPro is not established
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi unconnected");
        GoProHero4::link=GoProHero4::Disconnected;

      //if it is
      } else {
        //if it was not before
        if (GoProHero4::link!=GoProHero4::Connected) {
          GoProHero4::link=GoProHero4::Connected;
          Serial.println("WiFi connected");
          Serial.println("IP address: ");
          Serial.println(WiFi.localIP());
        }
        
        //prepare http request
        HTTPClient http;
        String url = "http://"+String(GoProHero4::host)+GoProURL;
        String HttpAnswer;
        http.begin(url);
      
        //execute http request
        Serial.println("GET "+url);  
        int httpCode = http.GET();
        
      
        //log http code returned
        Serial.print("Http code:");Serial.println(httpCode); 
        
        //Check the returned code
        //If WIFI OK but unable to connect
        if (httpCode==HTTPC_ERROR_CONNECTION_REFUSED) {
          //GOPRO seems to be OFF
          Serial.println("No GOPRO answer, seems to be OFF");
          power=Off;
  
        //else if http request result available
        } else if (httpCode > 0) { 
          //if HTTP OK received, GOPRO is ON
          if (httpCode==HTTP_CODE_OK) power=On;
          
          HttpAnswer = http.getString();   //Get the request response payload
          Serial.println(HttpAnswer);      //Print the response payload
      
          //Parse JSON answer in case of need
          if (httpCode==HTTP_CODE_OK or httpCode==HTTP_CODE_INTERNAL_SERVER_ERROR){
  
            //allocate memory for json deserialization (with help of http://arduinojson.org/v6/assistant to compute the capacity)
            DynamicJsonDocument GoProStatus(3000);
            
            DeserializationError error = deserializeJson(GoProStatus, HttpAnswer);
      
            //incase of error
            if (error) {
              Serial.print("DeserializeJson() failed: ");
              Serial.println(error.c_str());
            }
            //in case of success, refresh GoPro status
            else  GoProHero4::updateStatus(GoProStatus);
          }
        }
       
        http.end();
        return httpCode;
      }
    }


bool GoProHero4::isConnected() {
      //get WiFi Status
      //if WIFI connection is down
      if (WiFi.status()!= WL_CONNECTED){
        //if connection was up
        if ( link==Connected )  {
          link=Disconnected;
           Serial.println("Connection lost");
        }
        
      }

      //else if WIFI connection is up GoPro status still disconnected
      else if ( link==Disconnected ){
        //update GOPRO link status
        link=Connected;
        Serial.println("WIFI Connected");
        //refresh status (will happen only if GOPRO is On)
        refreshStatus();
      } 
      return ((bool)(link==Connected));
}

void GoProHero4::refreshStatus() {
      //get Gopro Status
      Serial.println("Get Status");
      ExecGoProCmd("/gp/gpControl/status");
}

void GoProHero4::photoMode() {
  //if video recording in progress
  if((currentMode==Video) && (recordingProcessing==Active)) {
    Serial.println("Stop Video");
    ExecGoProCmd("/gp/gpControl/command/shutter?p=0");
    //wait for end of recording before going on
     delay(1000);
  }
  //set mode to Photo
  Serial.println("Mode Photo");     
  ExecGoProCmd("/gp/gpControl/command/mode?p=1");

  //refresh status
  refreshStatus();
}
    
void GoProHero4::videoMode() {
      //set mode to Video
      Serial.println("Mode Video");     
      ExecGoProCmd("/gp/gpControl/command/mode?p=0");

      //refresh status
      refreshStatus();
    }

void GoProHero4::photoCapture() {
  //if mode Photo or video mode and recording Off

  if ((currentMode==Video) && (recordingProcessing==NotActive))  {
    Serial.println("Capture Video");
    ExecGoProCmd("/gp/gpControl/command/shutter?p=1");
  }
  else if((currentMode==Video) && (recordingProcessing==Active)) {
    Serial.println("Stop Video");
    ExecGoProCmd("/gp/gpControl/command/shutter?p=0");
    //wait for end of recording before going on
     delay(1000);
  }
  else if (currentMode==Photo){
    Serial.println("Capture Photo");
    ExecGoProCmd("/gp/gpControl/command/shutter?p=1");
  }
  else {
    Serial.println("Capture command not implemented in this mode");
  }
  //refresh status
  refreshStatus();
}

bool GoProHero4::powerOn() {

  //send WOL packet to the GOPRO
  WiFiUDP udp;
  byte preamble[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  byte i;
  
  Serial.println("Sending WOL packet"); 
  udp.beginPacket(GoProHero4::host,9); //sending packet at port 9, 
  udp.write(preamble, sizeof preamble);
  udp.write(GoProHero4::mac, 6);
  udp.endPacket();
   
  //wait for GOPRO to wake up. 1 second is sometime too short
  delay(1500);

  //refresh status
  refreshStatus();
  return(power==On);
}
