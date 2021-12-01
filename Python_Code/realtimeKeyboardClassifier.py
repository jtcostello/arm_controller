import sys
import glob
import serial
import time
import numpy as np
import keras
from collections import deque


'''
This script streams in EMG data over serial (wired or bluetooth), uses a neural network to classify grasp, and then
outputs keyboard strokes based on the grasp.

First run 'recordtrainingdata_fromESP32.py' to record data, then run 'trainNNClassifier.py' to train a classifier.
'''


# ---------------------------- Parameters ----------------------------
model_fname = 'nnmodel01'                                              # which model (classifier) to load
binwidth_samps = 25        # 5 = 100ms,   10 = 200ms,  25 = 500ms      # how many samples per classification bin

NUM_ELCTRODES = 8
SAMPLING_RATE = 50
# ---------------------------------------------------------------------



def get_serial_ports():
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            # s = serial.Serial(port)
            # s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def connectBTSerial():
    # print available ports
    ports = get_serial_ports()
    print("Choose the number of the serial port:")
    for (i, item) in enumerate(ports, start=0):
        print('\t',i, item)
    value = int(input())

    # connect serial
    ser = serial.Serial(ports[value], 115200)
    ser.flushInput()
    return ser



def readSerialEMG(ser):
    '''
    Reads in a received EMG sample containing data from 8 channels.
    For each time sample, the ESP32 should first send the startID (9999), followed by the 8 samples. The startID and
    each sample should be followed by a newline. (Serial.println(9999); Serial.println(electrode1); ...)
    '''

    while True:
        # wait for the start identifier
        startID_int = 9999
        ser_bytes = ser.readline()
        while (int(ser_bytes[0:len(ser_bytes) - 2].decode("utf-8")) != startID_int):
            ser_bytes = ser.readline()
            time.sleep(0.01)

        # now read in values
        emgsample = []
        for i in range(NUM_ELCTRODES):
            ser_bytes = ser.readline()
            emgsample[i] = int(ser_bytes[0:len(ser_bytes) - 2].decode("utf-8"))

        # print values
        print(emgsample)


class EmgStreamer:
    """
    Records EMG with the specified number of samples
    """
    def __init__(self, btserial, num_samps=50):
        self.num_samps = num_samps
        self.emg_data_queue = deque(maxlen=num_samps)
        self.btserial = btserial

    def clear_data(self):
        self.emg_data_queue.clear()

    def record_emg(self):

        # wait for the queue to fill (read in serial data)
        samples = 0
        while samples < self.num_samps:

            # wait for the start identifier
            startID_int = 9999
            ser_bytes = self.btserial.readline()
            while (int(ser_bytes[0:len(ser_bytes)-2].decode("utf-8")) != startID_int):
                ser_bytes = self.btserial.readline()
                time.sleep(0.01)

            # now read in values
            emgsample = []
            for i in range(NUM_ELCTRODES):
                ser_bytes = self.btserial.readline()
                emgsample[i] = int(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))

            # append to buffer
            self.emg_data_queue.append(emgsample)
            samples += 1

        # convert to np array
        return np.asarray(list(self.emg_data_queue))



def classify(model, emgbin):
    y_pred = model.predict(emgbin.reshape((1, -1)))
    y_pred_integer = np.argmax(y_pred, axis=1)
    return y_pred_integer


def typeKey(predictedclass):
    # TODO
    return




if __name__ == "__main__":

    # load trained classifier
    model = keras.models.load_model(model_fname)

    # setup serial
    btserial = connectBTSerial()
    emgstreamer = EmgStreamer(btserial, num_samps=binwidth_samps)

    while 1:
        # record emg
        emgarray = emgstreamer.record_emg()
        emgbin = emgarray.sum(axis=0)

        # run classifier
        predictedclass = classify(model, emgbin)
        print(predictedclass)

        # output keyboard stroke
        typeKey(predictedclass)

        # pause (might not be needed)
        time.sleep(0.01)












