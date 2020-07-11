#ifndef MAVLINKDECODER_H
#define MAVLINKDECODER_H

#include "mavlink/mavlink.h"

class MavLinkDecoder {
  private:
    bool failsafe=true;

  public:
    static const int MAX_FRAME_PERIOD=2000;  //above this delay, switch to failsafe
    uint16_t channel8=0;
    unsigned long int failSafeTimer;
  
    MavLinkDecoder();  
    void comm_receive(void);
    bool failSafe(void);

    //HOTT Ctrl typical value
    static const int CTRL_MIN=1100;
    static const int CTRL_M50=1300;
    static const int CTRL_CENTER=1500;
    static const int CTRL_P50=1700;
    static const int CTRL_MAX=1900;
    static const int CTRL_DEAD_ZONE=20;
};

#endif
