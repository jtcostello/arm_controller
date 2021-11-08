
#ifndef Constants_h
#define Constants_h

#include "Arduino.h"


#define NUM_EMG_HIST        50      // 5 = 100ms bins,  10 = 200ms bins,  25 = 500ms bins (should match python training)
#define NUM_ELECTRODES      8       // number of EMG sensors
#define NUM_BINS_PER_RUN    1       // how many bins are evaluated per each LDA run (usually 1)
#define NUM_CLASSES         1       // how many different hand grasps



// ************************** LDA Weights, copied from python output *******************************************
const float weights_neutral[8][1] = {
  {-0.012394661070092003},
  {0.022462159788915297},
  {0.013686619208086045},
  {-0.017505334494157882},
  {-0.011193745491098743},
  {-0.016792975210895702},
  {-0.029409649542919258},
  {-0.014052639664536813} };

const float intercepts_neutral[1] = {25.795272064608394};
 
const float weights_pinch[8][1] = {
  {0.0333286580950688},
  {-0.0801080479806958},
  {0.06952438644660397},
  {-0.07117683042909699},
  {-0.024902110761521514},
  {-0.017543179578720564},
  {0.013258316814736926},
  {0.03705691220676586} };

const float intercepts_pinch[1] = {17.555155950811056};
 
const float weights_mrp[8][1] = {
  {0.06774982630889083},
  {-0.03144520489301258},
  {-0.12282899203421697},
  {0.07412492301301855},
  {-0.02851612084983491},
  {-0.03258117134335445},
  {0.10652993903080579},
  {-0.06536772955718413} };

const float intercepts_mrp[1] = {14.151215553926683};
// *************************************************************************************************************


#endif
