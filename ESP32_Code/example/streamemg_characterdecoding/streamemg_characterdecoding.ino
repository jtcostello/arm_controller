
// includes
#include <map>
#include <myo.h>
#include "Constants.h"
#include "EMGStreamer.h"

// function prototypes
void emgCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
void setupMyo();
void decodeCharPulses(float bindata[NUM_ELECTRODES]);
void sendChar();

// globals
armband myo;
EMGStreamer emgstreamer = EMGStreamer();

// char decoding constants
const int shortlen = 80;      // [ms] min time for a short pulse
const int longlen = 250;      // [ms] min time for a long pulse
const int endcharlen = 400;   // [ms] min time for ending a character (time spent low)
const int threshold = 110;    //      threshold for detecting a pulse from emg
const bool DEBUG_CHAR = false;


/// MAKE SURE NUM_EMG_HIST = 1

// char decoding variables
int state = 0;
bool sentchar = false;
long wenthightime = 0;
long wentlowtime = 0;
uint8_t buff[5] = {0,0,0,0,0};
int buffpos = 0;

std::map<int, char> char_map = {
    {1000, 'e'}, //   e .
    {2000, 't'}, //   t -
    {1100, 'i'}, //   i ..
    {2100, 'n'}, //   n -.
    {1200, 'a'}, //   a .-
    {2200, 'm'}, //   m --
    {1110, 's'}, //   s ...
    {2110, 'd'}, //   d -.. 
    {1210, 'r'}, //   r .-.  
    {2210, 'g'}, //   g --.
    {2220, 'o'}, //   o ---
    {2120, 'k'}, //   k -.-
    {1120, 'u'}, //   u ..-
    {1220, 'w'}, //   w .--
    {2111, 'b'}, //   b -...
    {2121, 'c'}, //   c -.-.
    {1121, 'f'}, //   f ..-.
    {1111, 'h'}, //   h ....
    {1222, 'j'}, //   j .---
    {1211, 'l'}, //   l .-..
    {1221, 'p'}, //   p .--.
    {2212, 'q'}, //   q --.-
    {1112, 'v'}, //   v ...-
    {2112, 'x'}, //   x -..-
    {2122, 'y'}, //   y -.--
    {2211, 'z'}  //   z --..
};

std::map<int, char> char_map_special = {
    {1000, ' '}, //      space       .
    {2000, '|'}, //      backspace   -
    {1100, '.'} //    dot         ..
};






void setup() {
    Serial.begin(115200);
}


void loop() {
    if (!myo.connected) {
        setupMyo();

    } else {
        emgstreamer.binUpData();
        decodeCharPulses(emgstreamer.bindata);
        delay(10);
    }
}




void decodeCharPulses(float bindata[NUM_ELECTRODES]) {

    // sum up data, check if above threshold
    float val = 0;
    for (int i=0; i<NUM_ELECTRODES; i++) {
        val = val + bindata[i];
    }
    bool ishigh = (val > threshold);

    if (state) {
        // in the high state

        // if went low -> send a long or a short
        if (!ishigh) {
            long currenttime = millis();
            long hightime = currenttime - wenthightime;
            wentlowtime = currenttime;
            if (hightime > longlen) {
                // write a 2
                buff[buffpos] = 2;
                buffpos++;
                if (DEBUG_CHAR) Serial.print("-");
            } else if (hightime > shortlen) {
                // write a 1
                buff[buffpos] = 1;
                buffpos++;
                if (DEBUG_CHAR) Serial.print(".");
            } else {
                // not high for long enough
            }
            state = 0;

            if (buffpos>4) {
                sendChar();
                buffpos = 0;
                for (int j=0; j<5; j++) buff[j] = 0;
            }
        } else {
            // signal still high -> wait until it's low
        }

    } else {
        // in the low state

        if (ishigh) {
            // pulse started -> switch to high state
            state = 1;
            wenthightime = millis();
            sentchar = false;

        } else {
            // if low for long enough -> send character
            long currenttime = millis();
            long lowtime = currenttime - wentlowtime;
            if (lowtime > endcharlen) {
              if (!sentchar) {
                // send char & reset buffer
                sendChar();
                buffpos = 0;
                for (int j=0; j<5; j++) buff[j] = 0;
                if (DEBUG_CHAR) Serial.println("||");
                sentchar = true;
              }
            }
        }
    }
} // end decodeCharPulses


void sendChar() {
    // uses the global 'buff'

    char outchar;
    int char_hash = 1000*buff[1] + 100*buff[2] + 10*buff[3] + 1*buff[4];
    if (buff[0] == 2) {
        outchar = char_map_special[char_hash];
    } else {
        outchar = char_map[char_hash];
    }
    
    // output to computer
    Serial.print(outchar);
    // Test.comboKeyboard.write(outchar);

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
