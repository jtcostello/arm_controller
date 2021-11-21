


#include "Arduino.h"
#include <EEPROM.h>
#include "CentroidClassifier.h"
#include "Constants.h"
#include "EMGStreamer.h"


CentroidClassifier::CentroidClassifier() :
    training_sums{0}, training_sampcount{0}, class_means{0}{}



void CentroidClassifier::train(int training_num_samples, EMGStreamer &emgstreamer) {

    for (int j=0; j<NUM_CLASSES; j++) training_sampcount[j] = 0; // reset counts

    // loop over each class
    for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {

      Serial.print("Perform grasp\t\t");
      Serial.println(class_name_strings[thisclass]);
      delay(3000);
      Serial.println("...recording...");

      // record training data and save sums
      
      emgstreamer.resetCount();
      while (training_sampcount[thisclass] < training_num_samples) {

        // copy bin into the sum buffer when the bin has filled up
        if (emgstreamer.samplecounter >= NUM_EMG_HIST) {
            emgstreamer.pause();
            emgstreamer.binUpData();
            training_sampcount[thisclass] += 1;
            for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
              training_sums[thisclass][elec] += emgstreamer.bindata[elec];
            }
            emgstreamer.resetCount();
            emgstreamer.resume();
        }
        
        delay(10);
      }
    }

  // calculate means for each class
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
      class_means[thisclass][elec] = float(training_sums[thisclass][elec]) / float(training_sampcount[thisclass]);
    }
  }
}




int CentroidClassifier::predict(float bindata[NUM_ELECTRODES]) {

  // calc distance to each class (manhattan distance)
  float dist_sums[NUM_CLASSES] = {0};
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
      dist_sums[thisclass] += abs(bindata[elec] - class_means[thisclass][elec]);
    }
  }

  // find the nearest class
  float min_dist = 99999;
  int best_class = 0;
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    if (dist_sums[thisclass] < min_dist) {
      min_dist = dist_sums[thisclass];
      best_class = thisclass;
    }
  }
  return best_class;
}




void CentroidClassifier::loadWeights() {
  int eeAddress = 0;
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
      EEPROM.get(eeAddress, class_means[thisclass][elec]);
      eeAddress += 4; // each float is 4 bytes
    }
  }
}

void CentroidClassifier::saveWeights() {
  int eeAddress = 0;
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
      EEPROM.put(eeAddress, class_means[thisclass][elec]);
      eeAddress += 4; // each float is 4 bytes
    }
  }
}

void CentroidClassifier::printWeights() {
  for (int thisclass = 0; thisclass < NUM_CLASSES; thisclass++) {
    for (int elec = 0; elec < NUM_ELECTRODES; elec++) {
      Serial.print(class_means[thisclass][elec]);
      Serial.print('\t');
    }
    Serial.println(' ');
  }
}
