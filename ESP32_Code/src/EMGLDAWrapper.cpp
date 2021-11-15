#include "EMGLDAWrapper.h"

void myoLDAComboClass::setupMyo()
{
    Serial.println ("Trying to connect...");
    myo.connect();
    Serial.println (" - Connected");
    delay(1000);
    myo.set_myo_mode(myohw_emg_mode_send_emg,               // EMG mode: myohw_emg_mode_send_emg OR myohw_emg_mode_send_emg_raw
                    myohw_imu_mode_none,                   // IMU mode
                    myohw_classifier_mode_disabled);
    myo.emg_notification(TURN_ON)->registerForNotify(emgCallback); // setup the callback function
    myo.set_sleep_mode(myohw_sleep_mode_never_sleep); //**THIS HAS TO GO LAST**
}
uint8_t myoLDAComboClass::makeMyoPredictions()
{
    emgstreamer.binUpData();
    uint8_t handpos;
    uint8_t neutral = lda_isNeutral.predict(emgstreamer.bindata);
    if (neutral) 
    {
    handpos = 0;
    } 
    else 
    {
        uint8_t pinch = lda_pinch.predict(emgstreamer.bindata);
        uint8_t mrp = lda_mrp.predict(emgstreamer.bindata);
        if (pinch) 
        {
            handpos = mrp ? 3 : 2;
        } 
        else 
        {
            handpos = mrp ? 4 : 1;
        }
    }
    emgstreamer.resetCount();
    return handpos;
}
uint8_t myoLDAComboClass::debounceMyoPredictions()
{
    uint8_t count = 0;
    uint8_t output;
    uint8_t constant = makeMyoPredictions();
    while(count <= 20)
    {
        output = makeMyoPredictions();
        if(output == constant)
            count++;
        else
        {
            constant = output;
            count = 0;
        }
        delay(10);
    }
    return output;
}
int myoLDAComboClass::parse_gestures(uint8_t* gestures)
{
    uint8_t gesture_hash = (gestures[2] << 4) + (gestures[1] << 2) + gestures[0];
    return gesture_map[gesture_hash];
}
void myoLDAComboClass::bluetoothGestureSequence(uint8_t *buff)
{
    // Reset variables
    uint8_t last_prediction = 0;
    uint8_t lock;
    uint8_t gest = 0;
    memset(buff,0,(4*sizeof(*buff)));
    
    //MEMORY BUG FIX for Space ' '
    memset(buff,0,3);
    lockState(last_prediction);

    while (last_prediction != 4 && gest <= 2)
    {
    // Serial.println("Prepare Gesture");
    delay(1000);
    // Gather "arm" data
    last_prediction = debounceMyoPredictions();
    
    // Record "arm" data if is not the character or sequence delimiters
    if((last_prediction != 4) && (last_prediction != 0))
    {
        // Serial.print("Gesture Received: ");
        // Serial.println(last_prediction);
        buff[gest] = last_prediction;
        ++gest;
    }
    
    // Serial.println("Set hand to IDLE");
    lockState(last_prediction);
    }
}
void myoLDAComboClass::lockState(uint8_t current)
{
    // Wait until "arm" returns to neutral position
    while(current != 0)
    {
    // Serial.println("Locked");
    current = debounceMyoPredictions();
    }
    // Serial.println("Unlocked");
}
