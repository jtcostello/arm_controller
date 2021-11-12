


/*
  Gather's data from the serial bus and parses it into an output code.
*/

// includes
#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"
#include "OnlineLDA.h"
#include <stdio.h>

// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

// globals
uint8_t output;

// class objects
class myoLDAComboClass
{
  public:
    uint8_t buff[3] = {0,0,0};
    uint8_t gesture_map[65] = {
      0x20,     // 0,0,0 SPACE (This occurs if sequence Delimiter is the first gesture used)
      0x65,     // 1,0,0 e
      0x7F,     // 2,0,0, BACKSPACE
      0x61,     // 3,0,0 a
      0x00,     // 0,1,0
      0x72,     // 1,1,0 r
      0x74,     // 2,1,0 t
      0x6C,     // 3,1,0 l
      0x00,     // 0,2,0
      0x69,     // 1,2,0 i
      0x6E,     // 2,2,0 n
      0x63,     // 3,2,0 c
      0x00,     // 0,3,0
      0x6F,     // 1,3,0 o
      0x73,     // 2,3,0 s
      0xB7,     // 3,3,0 .
      0x00,     // 0,0,1
      0x00,     // 1,0,1
      0x00,     // 2,0,1
      0x00,     // 3,0,1
      0x00,     // 0,1,1
      0x62,     // 1,1,1 b
      0x71,     // 2,1,1 q
      0x21,     // 3,1,1 !
      0x00,     // 0,2,1
      0x67,     // 1,2,1 g
      0x77,     // 2,2,1 w
      0x22,     // 3,2,1 "
      0x00,     // 0,3,1
      0x6B,     // 1,3,1 k
      0x7A,     // 2,3,1 z
      0x26,     // 3,3,1 &
      0x00,     // 0,0,2
      0x00,     // 1,0,2
      0x00,     // 2,0,2
      0x00,     // 3,0,2
      0x00,     // 0,1,2
      0x64,     // 1,1,2 d
      0x75,     // 2,1,2 u
      0x3F,     // 3,1,2 ?
      0x00,     // 0,2,2
      0x68,     // 1,2,2 h
      0x78,     // 2,2,2 x
      0x3A,     // 3,2,2 :
      0x00,     // 0,3,2
      0x6D,     // 1,3,2 m
      0x0A,     // 2,3,2 Line Feed
      0x24,     // 3,3,2 $
      0x00,     // 0,0,3
      0x00,     // 1,0,3
      0x00,     // 2,0,3
      0x00,     // 3,0,3
      0x00,     // 0,1,3
      0x66,     // 1,1,3 f
      0x76,     // 2,1,3 v
      0x2C,     // 3,1,3 ,
      0x00,     // 0,2,3
      0x6A,     // 1,2,3 j
      0x79,     // 2,2,3 y
      0x3B,     // 3,2,3 ;
      0x00,     // 0,3,3
      0x70,     // 1,3,3 p
      0x0D,     // 2,3,3 Carriage Return
      0x27      // 3,3,3 '
    };
    armband myo;
    EMGStreamer emgstreamer;
    OnlineLDA lda_isNeutral;
    OnlineLDA lda_pinch;
    OnlineLDA lda_mrp;
    myoLDAComboClass()
    : myo{}, emgstreamer{}, lda_isNeutral(weights_neutral, intercepts_neutral), lda_pinch(weights_pinch, intercepts_pinch),lda_mrp(weights_mrp, intercepts_mrp)
    {
    }
    void setupMyo()
    {
      Serial.println ("Trying to connect...");
      myo.connect();
      Serial.println (" - Connected");
      delay(1000);
      myo.set_myo_mode(myohw_emg_mode_send_emg,               // EMG mode: myohw_emg_mode_send_emg OR myohw_emg_mode_send_emg_raw
                      myohw_imu_mode_none,                   // IMU mode
                      myohw_classifier_mode_disabled);
      myo.emg_notification(TURN_ON)->registerForNotify(emgCallback); // setup the callback function
      myo.set_sleep_mode(myohw_sleep_mode_never_sleep); //**THIS HAS TO GO LAST**
    }
    uint8_t makeMyoPredictions()
    {
      emgstreamer.binUpData();
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
      emgstreamer.resetCount();
      return handpos;
    }
    int parse_gestures(uint8_t* gestures)
    {
      uint8_t gesture_hash = (gestures[2] << 4) + (gestures[1] << 2) + gestures[0];
      return gesture_map[gesture_hash];
    }
    void bluetoothGestureSequence(uint8_t *buff)
    {
      // Reset variables
      uint8_t last_prediction = 0;
      uint8_t lock;
      uint8_t gest = 0;
      memset(buff,0,(4*sizeof(*buff)));

      while (last_prediction != 4 && gest <= 2)
      {
        // Serial.println("Prepare Gesture");
        delay(1000);
        // Gather "arm" data
        last_prediction = makeMyoPredictions();
        
        // Record "arm" data if is not the character or sequence delimiters
        if((last_prediction != 4) && (last_prediction != 0))
        {
          // Serial.print("Gesture Received: ");
          // Serial.println(last_prediction);
          buff[gest] = last_prediction;
          ++gest;
        }
        
        // Serial.println("Set hand to IDLE");
        lockState(last_prediction);
      }
    }
    void lockState(uint8_t current)
    {
      // Wait until "arm" returns to neutral position
      while(current != 0)
      {
        // Serial.println("Locked");
        current = makeMyoPredictions();
      }
      // Serial.println("Unlocked");
    }
};
myoLDAComboClass Test;

void setup() 
{
  Serial.begin(115200);
}

void loop() 
{
  if (!Test.myo.connected) 
    Test.setupMyo();
  else
  {
      /*Gather Predictions*/
      Test.bluetoothGestureSequence(Test.buff);
      /*Output Predictions*/
      Serial.print("Outputted Buffer: ");
      Serial.print(Test.buff[0]);
      Serial.print(Test.buff[1]);
      Serial.println(Test.buff[2]);
      /*Format and output predictions into new value*/
      output = Test.parse_gestures(Test.buff);
      Serial.print("Outputted Bluetooth Keypress: ");
      Serial.println((char)output);
  }
} 

void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
  Test.emgstreamer.streamData(pData, length);
}
