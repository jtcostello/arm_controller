


/*
  Gather's data from the serial bus and parses it into an output code.
*/

// includes
#include <EMGLDAWrapper.h>

// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

// globals
uint8_t output;

// class objects
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

