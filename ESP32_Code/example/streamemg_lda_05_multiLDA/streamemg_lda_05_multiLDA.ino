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
#include <stdio.h>


// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void graphicalPrintClassifier(int finger_class);
void setupMyo();
int makePredictions();
int parseGestures(uint8_t* gestures);


// class objects
armband myo;
EMGStreamer emgstreamer = EMGStreamer();
OnlineLDA lda_isNeutral   = OnlineLDA(weights_neutral, intercepts_neutral);
OnlineLDA lda_pinch       = OnlineLDA(weights_pinch, intercepts_pinch);
OnlineLDA lda_mrp         = OnlineLDA(weights_mrp, intercepts_mrp);

// globals
int output;
int buffer[3];
// 0 -> idle
// 1 -> gesture #1
// 2 -> gesture #2
// 3 -> gesture #3
// 4 -> sen
char gesture_map[] = {
  0x41,     // 1,0,0, A
  0x20,     // 2,0,0, SPACE
  0x7F,     // 3,0,0, BACKSPACE
  0x00,     // 1,1,0
  0x00,     // 1,2,0
  0x00,     // 1,3,0
  0x00,     // 2,1,0
  0x00,     // 2,2,0
  0x00,     // 2,3,0
  0x00,     // 3,1,0
  0x00,     // 3,2,0
  0x00,     // 3,3,0
  0x00,     // 1,1,1
  0x00,     // 1,1,2
  0x00,     // 1,1,3
  0x00,     // 1,2,1
  0x00,     // 1,2,2
  0x00,     // 1,2,3
  0x00,     // 1,3,1
  0x00,     // 1,3,2
  0x00,     // 1,3,3
  0x00,     // 2,1,1
  0x00,     // 2,1,2
  0x00,     // 2,1,3
  0x00,     // 2,2,1
  0x00,     // 2,2,2
  0x00,     // 2,2,3
  0x00,     // 2,3,1
  0x00,     // 2,3,2
  0x00,     // 2,3,3
  0x00,     // 3,1,1
  0x00,     // 3,1,2
  0x00,     // 3,1,3
  0x00,     // 3,2,1
  0x00,     // 3,2,2
  0x00,     // 3,2,3
  0x00,     // 3,3,1
  0x00,     // 3,3,2
  0x00      // 3,3,3
};


void setup() {
  Serial.begin(115200);
}


void loop() {

  if (!myo.connected) {
    setupMyo();

  } else {
    Serial.println("Get ready to input gesture sequence\n");
    delay(1000);
    gestureSequence();
    output = parse_gestures(buffer);
    Serial.println(output, HEX);
  }

} // end main loop



int makePredictions() {
  int handpos;
  int neutral = lda_isNeutral.predict(emgstreamer.bindata);
  if (neutral) 
  {
    handpos = 0;
  } 
  else 
  {
    int pinch = lda_pinch.predict(emgstreamer.bindata);
    int mrp = lda_mrp.predict(emgstreamer.bindata);
    if (pinch) 
    {
      handpos = mrp ? 3 : 2;
    } 
    else 
    {
      handpos = mrp ? 4 : 1;
    }
  }
  // graphicalPrintClassifier(handpos);
  return handpos;
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

void gestureSequence()
{
  int last_prediction = 0;
  memset(buffer, 0, 3);
  uint8_t gest = 0;
  
  while (last_prediction != 4 || gest < 3)
  {
    emgstreamer.binUpData();
    last_prediction = makePredictions();
    emgstreamer.resetCount();
    delay(1000);

    int curr_prediction = 1;
    while(curr_prediction != 0)
    {
      emgstreamer.binUpData();
      curr_prediction = makePredictions();
      emgstreamer.resetCount();
      delay(50);
      // Serial.print("Gesture Received ");
      // Serial.println(curr_prediction); 
    }
    
    if(last_prediction != 4)
    {
      Serial.print("Gesture Received ");
      Serial.println(last_prediction);
      buffer[gest] = last_prediction;
      ++gest;
    }
  }
  Serial.println("Buffer Sent");
}

// assumes each gesture takes the value 0-3
// otherwise this function will break
// gestures[0] is assumed to be the first gesture in the sequence
// value 0 is assumed to be the idle gesture
// gestures[0] cannot be 0
// gestures[1] cannot be 0 if gestures[2] != 0, but can be 0 if gestures[2] == 0
// gestures[2] can be 0
int parse_gestures(int* gestures){
  assert(gestures[0] != 0);
  assert(gestures[1] != 0 || gestures[2] == 0);
  // bit field, {gestures[2], gestures[1], gestures[0]} -> 6 bit number
  int gesture_hash = (gestures[2] << 4) + (gestures[1] << 2) + gestures[0];

  // reads from a gesture_map, which has values from index 0 - 38
  // it is assumed that gesture_hash cannot be greater than 38 due to assumptions above
  return (int)gesture_map[gesture_hash];
}
