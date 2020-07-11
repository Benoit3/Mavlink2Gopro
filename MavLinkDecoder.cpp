//Library for Decoding Mavlink written with the help of:
//https://discuss.ardupilot.org/t/mavlink-and-arduino-step-by-step/25566

#include <ESP8266WiFi.h>
#include "MavLinkDecoder.h"

MavLinkDecoder::MavLinkDecoder() {
  failSafeTimer=millis();
}
    
void MavLinkDecoder::comm_receive() {
  mavlink_message_t msg;
  mavlink_status_t status;

  
  while(Serial.available()>0) {
	uint8_t c = Serial.read();
	
	// Try to get a new message
	if(mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
	  // Handle message
	  switch(msg.msgid) {
		case MAVLINK_MSG_ID_HEARTBEAT:  // #0: Heartbeat
		  {
			// E.g. read GCS heartbeat and go into
			// comm lost mode if timer times out
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		  }
		  break;


		case MAVLINK_MSG_ID_RC_CHANNELS_RAW:  // #35: RAW_CHANNELS
		  {
			/* getting channel8 position */
			channel8=mavlink_msg_rc_channels_raw_get_chan8_raw(&msg);
		  }
		  break;

		 case MAVLINK_MSG_ID_SYS_STATUS:  // #1: SYS_STATUS
		  {
			/* getting failsafe status*/
			failsafe=( (mavlink_msg_sys_status_get_onboard_control_sensors_health(&msg)& 0x10000)==0);
			failSafeTimer=millis();
		  }
		  break;
				
	   default:
		  break;
	  }
	}
  }
  
}

bool MavLinkDecoder::failSafe(void) {
  if (millis()-failSafeTimer < MAX_FRAME_PERIOD) return(failsafe);
  else return(true);
  }
