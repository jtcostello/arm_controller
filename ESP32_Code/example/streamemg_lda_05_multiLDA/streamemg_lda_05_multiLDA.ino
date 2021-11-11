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
int parseGestures(uint8_t* gestures);
uint8_t makeSerialPredictions();

// globals
uint8_t output;
uint8_t buff[3] = {0,0,0};
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


void setup() 
{
  Serial.begin(115200);
}

void loop() 
{
  serialGestureSequence(buff);
  Serial.print("Outputted Buffer: ");
  Serial.print(buff[0]);
  Serial.print(buff[1]);
  Serial.println(buff[2]);
  output = parse_gestures(buff);
  Serial.print("Outputted parsed Gesture: ");
  Serial.println(output,HEX);
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

// assumes each gesture takes the value 0-3
// otherwise this function will break
// gestures[0] is assumed to be the first gesture in the sequence
// value 0 is assumed to be the idle gesture
// gestures[0] cannot be 0
// gestures[1] cannot be 0 if gestures[2] != 0, but can be 0 if gestures[2] == 0
// gestures[2] can be 0
int parse_gestures(uint8_t* gestures)
{
//  assert(gestures[0] != 0);
//  assert(gestures[1] != 0 || gestures[2] == 0);
  // bit field, {gestures[0], gestures[1], gestures[2]} -> 6 bit number
  uint8_t gesture_hash = (gestures[0] << 4) + (gestures[1] << 2) + gestures[2];

  // reads from a gesture_map, which has values from index 0 - 38
  // it is assumed that gesture_hash cannot be greater than 38 due to assumptions above
  return gesture_hash;
//  return (int)gesture_map[gesture_hash];
}
