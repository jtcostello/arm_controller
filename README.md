# Overview
Library to stream in MYO armband EMG data and classify 5 gestures in real time. ML Training is run in python, then exported to c++ code to run on an ESP32.

# Libraries Used In the Creation of this Code
### Arduino Libraries
Refer to the following repository, and it's installation reference, in order to get this repository's library functional:
Sparthan-myo library: (https://github.com/project-sparthan/sparthan-myo) (ver 0.1.0)

Refer to the following forum post, in order to install constants.h (constants.h is a file in this library): (https://forum.arduino.cc/t/can-i-include-a-header-file-that-is-not-a-library/37468)

Using Arduino IDE and arduino.h for programming the ESP32 (https://github.com/espressif/arduino-esp32) (ver 1.0.6)

Install it by following instructions in the github link.

### Python Libraries
Python libraries needed: myo (see below), numpy, pandas, matplotlib, sklearn  
Install 'myo' using 'pip install myo-python'

Myo connect software is required for the python bluetooth connection (software is no longer actively supported): https://myo-connect.software.informer.com/download/

# Installation Instructions
As mentioned above, first follow each repo's respecive instructions for their proper installations. Then, download the files within the "src" folder. Lastly, put the files witnin the "src" folder into a .zip file and use the following link to install the library: (https://www.arduino.cc/en/guide/libraries#toc4). When using this repo, use the src file from Tag2 for the most up to date code.

Install python libaries using pip.

# Utilizing the Example Code
Regarding hardware, only a [MYO armband](https://www.robotshop.com/en/myo-gesture-control-armband-black.html) and an ESP32 are needed.

### Python Visualization, Data Collection, and ML Training
1. Connect to the Myo using Myo Connect software
2. (optional) Verify Bluetooth connection and visualize code by running `realtime_plot_emg_smooth.py`
3. Use `recordtrainingdata.py` to record data. The program tells the user which grasp to perform, and records data. Set the output filename at the top.
4. Use `exportLDAclassifier_multiLDA.py` to process data and train LDA classifier weights. The program outputs 3 sets of weights for the neutral, pinch, and MRP classifiers.

### ESP32 Code
5. Copy the weight matrices (from step 4) into `constants.h`
6. Compile & run on ESP32. It will try to connect to the Myo over Bluetooth, and once connected will make grasp classifications and print to serial.

