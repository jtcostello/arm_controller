
#ifndef OnlineLDA_h
#define OnlineLDA_h

#include "Arduino.h"
#include "Constants.h"


class OnlineLDA {
    public:
        OnlineLDA(const float (&weightsin)[NUM_ELECTRODES][NUM_CLASSES], const float (&interceptsin)[NUM_CLASSES]);
        int predict(float bindata[NUM_ELECTRODES]);

    private:

        // references to weights
        const float (&weights)[NUM_ELECTRODES][NUM_CLASSES];
        const float (&intercepts)[NUM_CLASSES];

        // matrix dimensions
        const int r1;
        const int c1;
        const int r2;
        const int c2;

        // matrices for LDA computations
        float outmat[NUM_BINS_PER_RUN][NUM_CLASSES];    // stores the result of LDA matrix multiplication
        int yhat[NUM_BINS_PER_RUN];                     // stores the LDA prediction (value = 0..NUM_CLASSES-1)

};


#endif







