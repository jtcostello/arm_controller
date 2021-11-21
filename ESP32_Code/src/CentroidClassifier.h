
#ifndef CentroidClassifier_h
#define CentroidClassifier_h

#include "Arduino.h"
#include "Constants.h"
#include "EMGStreamer.h"


class CentroidClassifier {
  
    public:
    
        CentroidClassifier();
        int predict(float bindata[NUM_ELECTRODES]);
        void loadWeights();
        void saveWeights();
        void printWeights(); 
        void train(int training_num_samples, EMGStreamer &emgstreamer);

    private:
        // arrays to store training and mean info
        int training_sums[NUM_CLASSES][NUM_ELECTRODES];
        int training_sampcount[NUM_CLASSES];
        float class_means[NUM_CLASSES][NUM_ELECTRODES];
};


#endif
