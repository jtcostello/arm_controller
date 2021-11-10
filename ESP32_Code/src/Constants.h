
#ifndef Constants_h
#define Constants_h

#include "Arduino.h"


#define NUM_EMG_HIST        25      // 5 = 100ms bins,  10 = 200ms bins,  25 = 500ms bins (should match python training)
#define NUM_ELECTRODES      8       // number of EMG sensors
#define NUM_BINS_PER_RUN    1       // how many bins are evaluated per each LDA run (usually 1)
#define NUM_CLASSES         1       // how many different hand grasps



// ************************** LDA Weights, copied from python output *******************************************
const float weights_neutral[8][1] = {
    {-0.035893321296201076},
    {-0.023884334729241674},
    {-0.023549785751556564},
    {-0.05116798931052901},
    {0.06204311199269302},
    {-0.0341351951162587},
    {-0.03739261979975168},
    {-0.0032148098541800634} };

const float intercepts_neutral[1] = {38.06482966621526};
const float weights_pinch[8][1] = {
    {-0.05278573921288337},
    {-0.014025711987733242},
    {-0.029041619643278786},
    {-0.0817033180157383},
    {0.006245031750209126},
    {0.04639988727787302},
    {0.030804122583261147},
    {0.05745978787078425} };

const float intercepts_pinch[1] = {25.476903513809077};
const float weights_mrp[8][1] = {
    {-0.007167858565116108},
    {0.019297788423556443},
    {-0.049456758618220395},
    {0.01719422649964793},
    {0.023933812136784464},
    {-0.09679275300995586},
    {0.041588995496298395},
    {-0.009064648577117473} };

const float intercepts_mrp[1] = {16.493949028855383};
// *************************************************************************************************************


#endif
