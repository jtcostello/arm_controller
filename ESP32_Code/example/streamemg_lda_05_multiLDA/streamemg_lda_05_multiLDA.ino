


/*
  Gather's data from the serial bus and parses it into an output code.
*/

// includes
#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"
#include "OnlineLDA.h"
#include <stdio.h>
#include <EECS473BLECombo.h>


// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void setupMyo();
int parseGestures(uint8_t* gestures);
uint8_t makeSerialPredictions();
uint8_t makeMyoPredictions();

// globals
uint8_t output;
uint8_t buff[3] = {0,0,0};
// 0 -> idle
// 1 -> gesture #1
// 2 -> gesture #2
// 3 -> gesture #3
// 4 -> sen
uint8_t gesture_map[] = {
  0x41,     // 0,0,0, A
  0x20,     // 1,0,0, SPACE
  0x7F,     // 2,0,0, BACKSPACE
  0x42,     // 3,0,0, B
  0x00,     // 0,1,0
  0x43,     // 1,1,0
  0x44,     // 2,1,0
  0x45,     // 3,1,0
  0x00,     // 0,2,0
  0x46,     // 1,2,0
  0x47,     // 2,2,0
  0x48,     // 3,2,0
  0x00,     // 0,3,0
  0x49,     // 1,3,0
  0x4A,     // 2,3,0
  0x4B,     // 3,3,0
  0x00,     // 0,0,1
  0x00,     // 1,0,1
  0x00,     // 2,0,1
  0x00,     // 3,0,1
  0x00,     // 0,1,1
  0x4C,     // 1,1,1
  0x4D,     // 2,1,1
  0x4E,     // 3,1,1
  0x00,     // 0,2,1
  0x4F,     // 1,2,1
  0x50,     // 2,2,1
  0x51,     // 3,2,1
  0x00,     // 0,3,1
  0x52,     // 1,3,1
  0x53,     // 2,3,1
  0x54,     // 3,3,1
  0x00,     // 0,0,2
  0x00,     // 1,0,2
  0x00,     // 2,0,2
  0x00,     // 3,0,2
  0x00,     // 0,1,2
  0x55,     // 1,1,2
  0x56      // 2,1,2
  
};

// class objects
armband myo;
EMGStreamer emgstreamer = EMGStreamer();
OnlineLDA lda_isNeutral   = OnlineLDA(weights_neutral, intercepts_neutral);
OnlineLDA lda_pinch       = OnlineLDA(weights_pinch, intercepts_pinch);
OnlineLDA lda_mrp         = OnlineLDA(weights_mrp, intercepts_mrp);
BLEClass Test;

void setup() 
{
  Serial.begin(115200);
  byte status = Test.init();
  while(status != 0)
    Serial.println("Cant connect to MPU");
}

void loop() 
{
  if (!myo.connected) 
    setupMyo();
  else
  {
    emgstreamer.binUpData();
    output =  makeMyoPredictions();
    Serial.println(output);
    delay(100);
    emgstreamer.resetCount();
  }
  // if(Test.comboKeyboard.isConnected()) 
  // {
  //   serialGestureSequence(buff);
  //   Serial.print("Outputted Buffer: ");
  //   Serial.print(buff[0]);
  //   Serial.print(buff[1]);
  //   Serial.println(buff[2]);
  //   output = parse_gestures(buff);
  //   Serial.print("Outputted Bluetooth Keypress: ");
  //   Serial.println((char)output);
  //   Test.comboKeyboard.write((char)output);
  // }
} // end main loop

uint8_t makeSerialPredictions()
{
  uint8_t v;
  // Wait for a single byte to come in
  Serial.println("Waiting for input");
  while(!Serial.available());
  // Record integer version of input
  v = (Serial.read() - 48);
  if(v > 4)
    return 0;
  else 
    return v;
}

uint8_t makeMyoPredictions()
{
  uint8_t handpos;
  uint8_t neutral = lda_isNeutral.predict(emgstreamer.bindata);
  if (neutral) 
  {
    handpos = 0;
  } 
  else 
  {
    uint8_t pinch = lda_pinch.predict(emgstreamer.bindata);
    uint8_t mrp = lda_mrp.predict(emgstreamer.bindata);
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
void serialGestureSequence(uint8_t *buff)
{
  // Reset variables
  uint8_t last_prediction = 0;
  uint8_t lock;
  uint8_t gest = 0;
  memset(buff,0,(4*sizeof(*buff)));

  while (last_prediction != 4 && gest <= 2)
  {
    // Gather "arm" data
    last_prediction = makeSerialPredictions();
    lock = last_prediction;
    // Record "arm" data if is not the character or sequence delimiters
    if((last_prediction != 4) && (last_prediction != 0))
    {
      Serial.print("Gesture Received: ");
      Serial.println(last_prediction);
      buff[gest] = last_prediction;
      ++gest;
    }
    // Wait until "arm" returns to neutral position
    while(lock != 0)
    {
      Serial.println("Locked");
      lock = makeSerialPredictions();
    }
    Serial.println("Unlocked");
  }
}

int parse_gestures(uint8_t* gestures)
{
  uint8_t gesture_hash = (gestures[2] << 4) + (gestures[1] << 2) + gestures[0];
  return gesture_map[gesture_hash];
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