/*
  J Costello
  Streams EMG from Myo and predicts hand grasp using LDA. Graphically prints classifier output to serial.
  LDA weights should by trained and copied from python.
  This version uses the EMGStreamer and OnlineLDA classes.
*/

// includes
#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"
#include "OnlineLDA.h"
//#include "C:\Users\Joey\Dropbox (University of Michigan)\EECS 473\Project\arduino code\EMGStreamer.h"
//#include "C:\Users\Joey\Dropbox (University of Michigan)\EECS 473\Project\arduino code\OnlineLDA.h"


// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void graphicalPrintClassifier(int finger_class);
void setupMyo();


// class objects
armband myo;
EMGStreamer emgstreamer = EMGStreamer();
OnlineLDA lda = OnlineLDA(weights, intercepts);



void setup() {
  Serial.begin(115200);
}


void loop() {

  if (!myo.connected) {
    setupMyo();

  } else {
    // run LDA
    emgstreamer.binUpData();
    int prediction = lda.predict(emgstreamer.bindata);
    graphicalPrintClassifier(prediction);
    emgstreamer.resetCount();
    delay(200);
  }

} // end main loop



void setupMyo() {
  Serial.println ("Trying to connect...");
  myo.connect();
  Serial.println (" - Connected");
  delay(100);
  myo.set_myo_mode(myohw_emg_mode_send_emg,               // EMG mode: myohw_emg_mode_send_emg OR myohw_emg_mode_send_emg_raw
                   myohw_imu_mode_none,                   // IMU mode
                   myohw_classifier_mode_disabled);
  myo.emg_notification(TURN_ON)->registerForNotify(emgCallback); // setup the callback function
  myo.set_sleep_mode(myohw_sleep_mode_never_sleep); //**THIS HAS TO GO LAST**
}



void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  emgstreamer.streamData(pData, length);
}




void graphicalPrintClassifier(int finger_class) {
  switch (finger_class) {
    case 0:
      // hand open
      Serial.println();
      Serial.println();
      Serial.println("    _    ");
      Serial.println("   | |");
      Serial.println("---| |____________");
      Serial.println("          ________)");
      Serial.println("          _________)");
      Serial.println("         _________)");
      Serial.println("---.____________)");
      break;

    case 1:
      // thumb pinch
      Serial.println();
      Serial.println();
      Serial.println("    _____");
      Serial.println("---'   __)__");
      Serial.println("      (_____)______");
      Serial.println("          _________)");
      Serial.println("         _________)");
      Serial.println("---.____________)");
      break;

    case 2:
      // closed hand
      Serial.println();
      Serial.println();
      Serial.println("    _____");
      Serial.println("---'   __)__");
      Serial.println("      (_____)");
      Serial.println("      (_____)");
      Serial.println("      (____)");
      Serial.println("---.__(___)");
      break;

    case 3:
      // mrp closed
      Serial.println();
      Serial.println();
      Serial.println("    _    ");
      Serial.println("   | |");
      Serial.println("---| |____________");
      Serial.println("       ___________)");
      Serial.println("      (_____)");
      Serial.println("      (____)");
      Serial.println("---.__(___)");
      break;
  }
}
