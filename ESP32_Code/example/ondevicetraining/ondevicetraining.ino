
// On-Device Training Code
// J Costello
//
// The `train` function instructs on performing each grasp, saves emg data, and trains the classifier.
// `saveWeights` saves the current weights to EEPROM memory.


// includes
#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"
#include "CentroidClassifier.h"


// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void setupMyo();
void graphicalPrintClassifier(int finger_class);

// class objects
armband myo;
EMGStreamer emgstreamer = EMGStreamer();
CentroidClassifier classifier = CentroidClassifier();

// training constants
const int training_num_samples = 8; // how many bins to average over (bins are usually ~500ms)

// *** Make sure NUM_CLASSES=5 in constants.h***


void setup() {
  Serial.begin(115200);
}


void loop() {

  if (!myo.connected) {
    setupMyo();

  } else {

    classifier.train(training_num_samples, emgstreamer);
    classifier.saveWeights();
    classifier.printWeights();
    classifier.loadWeights();

    while(1) {
        // run classifier
        emgstreamer.binUpData();
        int prediction = classifier.predict(emgstreamer.bindata);
        graphicalPrintClassifier(prediction);
        delay(200);
    }
  }

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
