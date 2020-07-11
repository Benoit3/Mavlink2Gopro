#include <ArduinoJson.h>

class GoProHero4 {
  private:
    static const char* host;
    void updateStatus(DynamicJsonDocument GoProStatus);
    int ExecGoProCmd(String GoProURL);

  public:
    static byte *mac;
    enum linkValue {Disconnected=0,Connected=1} link;
    enum PowerValue {Off=0,On=1} power;
    enum ModeValue {Video=0,Photo=1,MultiShot=2,Read=4,Setup=5} currentMode;
    enum RecordingProcessingState {NotActive=0,Active=1} recordingProcessing;
    GoProHero4(char* ,char*,byte* mac);
    bool isConnected();
    void refreshStatus();
    void photoMode();
    void videoMode();
    void photoCapture();
    bool powerOn();
};
