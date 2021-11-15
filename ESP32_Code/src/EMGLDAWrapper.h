#ifndef EMGLDAWrapper_h
#define EMGLDAWrapper_h

#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"
#include "OnlineLDA.h"
#include <stdio.h>

void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

class myoLDAComboClass
{
  public:
    uint8_t buff[3] = {0,0,0};
    uint8_t gesture_map[65] = {
      0x20,     // 0,0,0 SPACE (This occurs if sequence Delimiter is the first gesture used)
      0x65,     // 1,0,0 e
      0x08,     // 2,0,0, BACKSPACE
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
    void setupMyo();
    uint8_t makeMyoPredictions();
    int parse_gestures(uint8_t* gestures);
    void bluetoothGestureSequence(uint8_t *buff);
    void lockState(uint8_t current);
};
#endif
