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


// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void graphicalPrintClassifier(int finger_class);
void setupMyo();
int makePredictions();


// class objects
armband myo;
EMGStreamer emgstreamer = EMGStreamer();
OnlineLDA lda_isNeutral   = OnlineLDA(weights_neutral, intercepts_neutral);
OnlineLDA lda_pinch       = OnlineLDA(weights_pinch, intercepts_pinch);
OnlineLDA lda_mrp         = OnlineLDA(weights_mrp, intercepts_mrp);



void setup() {
  Serial.begin(115200);
}


void loop() {

  if (!myo.connected) {
    setupMyo();

  } else {
    // run LDA
    emgstreamer.binUpData();
    makePredictions();
//    graphicalPrintClassifier(prediction);
    emgstreamer.resetCount();
    delay(200);
  }

} // end main loop



int makePredictions() {
  int neutral = lda_isNeutral.predict(emgstreamer.bindata);
//  if (neutral) {
//    Serial.println("NNNNNNNNNNN");
//  } else {
//    Serial.println("..");
//  }
  int pinch = lda_pinch.predict(emgstreamer.bindata);
  int mrp = lda_mrp.predict(emgstreamer.bindata);
  
  int handpos = 0;
  if (pinch) {
    handpos = mrp ? 3 : 2;
  } else {
    handpos = mrp ? 4 : 1;
  }
  
  graphicalPrintClassifier(handpos);

  return 0;
  
}





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
    case 1:
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

    case 2:
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

    case 3:
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

    case 4:
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
