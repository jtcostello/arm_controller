


#include "Arduino.h"
#include "OnlineLDA.h"
#include "Constants.h"

OnlineLDA::OnlineLDA(const float (&weightsin)[NUM_ELECTRODES][NUM_CLASSES], const float (&interceptsin)[NUM_CLASSES]) :
    r1(NUM_BINS_PER_RUN), c1(NUM_ELECTRODES), r2(NUM_ELECTRODES), c2(NUM_CLASSES),
     outmat{0}, yhat{0}, weights(weightsin), intercepts(interceptsin) {}


int OnlineLDA::predict(float bindata[NUM_ELECTRODES]) {
  // (num_bins x 8) * (8 x num_classes) = (num_bins x num_classes)

  // outmat = X*weights
  for (int i = 0; i < r1; i++) {
    for (int j = 0; j < c2; j++) {
      outmat[i][j] = 0;
      for (int k = 0; k < r2; k++) {
        //        outmat[i][j] += (m1[i][k] * m2[k][j]);
        outmat[i][j] += (bindata[k] * weights[k][j]);
      }
    }
  }

  // outmat = outmat + intercepts
  for (int i = 0; i < r1; i++) {
    for (int j = 0; j < c2; j++) {
      outmat[i][j] += intercepts[j];
    }
  }


  if (NUM_CLASSES == 1) {
    
    // special binary decision case:
    return (outmat[0][0]>0) ? int(1) : int(0);  
    
  } else {
    
    // multiclass decision:
    // yhat = argmax(outmat, dim=rows)
    for (int i = 0; i < r1; i++) {
      float maxval = outmat[i][0];
      yhat[i] = 0;
      for (int j = 0; j < c2; j++) {
        if (outmat[i][j] > maxval) {
          maxval = outmat[i][j];
          yhat[i] = j;
        }
      }
    }
    return yhat[0];
    
  }

  

  //  //-----------------------------
  //  // DEBUG: print outmat
  //  for (int i = 0; i < r1; i++) {
  //    for (int j = 0; j < c2; j++) {
  //      Serial.print(outmat[i][j]); Serial.print("\t");
  //    }
  //    Serial.println();
  //  }
  //  //------------------------------

  

}
