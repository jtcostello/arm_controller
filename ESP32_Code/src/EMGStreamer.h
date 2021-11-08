
#ifndef EMGStreamer_h
#define EMGStreamer_h

#include "Arduino.h"
#include "Constants.h"

class EMGStreamer {
    public:
        EMGStreamer();
        void resetCount();
        void binUpData();
        void streamData(uint8_t* pData, size_t length);

        float bindata[NUM_ELECTRODES];                          // stores binned data, used as input to LDA
        volatile int emgdata[NUM_EMG_HIST][NUM_ELECTRODES];     // stores recent EMG data (circular buffer)

        volatile int bufferpos;             // current zero-idx of the circular buffer (where we will overwrite next)
        volatile int samplecounter;
        const int num_electrodes;
        const int num_emg_hist;
};

#endif
