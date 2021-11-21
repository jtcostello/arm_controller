

#include "Arduino.h"
#include "EMGStreamer.h"


EMGStreamer::EMGStreamer() :
    num_electrodes(NUM_ELECTRODES), num_emg_hist(NUM_EMG_HIST), bufferpos(0), samplecounter(0),
    bindata{0}, emgdata{0}, is_paused(false) {}


void EMGStreamer::resetCount() {
    samplecounter = 0;
}

void EMGStreamer::pause() {
    is_paused = true;
}

void EMGStreamer::resume() {
    is_paused = false;
}


void EMGStreamer::binUpData() {
  // sum up the current data stored in the emg buffer into a bin
  for (int elec_idx = 0; elec_idx < num_electrodes; elec_idx++) {
    bindata[elec_idx] = 0;
    for (int j = 0; j < num_emg_hist; j++) {
      bindata[elec_idx] += emgdata[j][elec_idx];
    }
  }
}


void EMGStreamer::streamData(uint8_t* pData, size_t length_in) {
  if (~is_paused) {
    // at each callback, we get two samples per electrode, i.e. [elec1,...elec8, elec1,...elec8]
    for (int i = 0; i < length_in; i++) {
  
      // take absolute value of emg reading
      int x = int(pData[i]);
      int absx = (x > 127) ? abs(x - 255) : abs(x);
  
      // save data
      int elec_idx = (i < 8) ? i : i - 8;
      emgdata[bufferpos][elec_idx] = absx;
  
      // increment the buffer position if this is the last electrode
      if (i == 7 | i == 15) {
        bufferpos++;
        if (bufferpos >= num_emg_hist) {
          bufferpos = 0;
        }
        samplecounter++;
      }
    }
  }
}
