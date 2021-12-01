from collections import deque
import pickle
import time
import datetime
import sys
import glob
import serial
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

'''
Streams in EMG data over serial (wired or bluetooth) from an ESP32 connected to a Myo armband.
For each data sample, the ESP32 should first send the startID (9999), followed by the 8
samples. The startID and each sample should be followed by a newline.

Recorded data is saved to file, as specified by the 'out_fname' variable.
'''


# ------ Recording parameters ----------
out_fname = 'emgdata08.pkl'     # the filename of where to save the recorded data
seconds_per_grasp = 4           # how many seconds of data to record per grasp
num_repeats = 4                 # how many times to cycle through all grasps
show_gesture_image = False      # if we should show the gesture images (must be in same directory)

finger_groups = ['pinch', 'MRP']
postures = [[0, 'neutral', 'neutral'], [1, 'open', 'open'], [2, 'close', 'open'], [3, 'close', 'close'], [4, 'open', 'close']]


NUM_ELCTRODES = 8
SAMPLING_RATE = 50

# --------------------------------------  



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
        print('\t', i, item)
    value = int(input())

    # connect serial
    ser = serial.Serial(ports[value], 115200)
    ser.flushInput()
    return ser


def showGestureImage(gesturenum, text):
    img = mpimg.imread(f'gesture{gesturenum}.jpg')
    imgplot = plt.imshow(img)
    ax = plt.gca()
    ax.set_title(text)
    ax.axes.xaxis.set_visible(False)
    ax.axes.yaxis.set_visible(False)
    plt.show(block=False)
    plt.pause(0.01)  # required for plot to show



class EmgRecorder():
    """
    Records EMG with the specified number of samples
    """
    def __init__(self, btserial, num_samps=50):
        self.num_samps = num_samps
        self.emg_data_queue = deque(maxlen=num_samps+1)
        self.btserial = btserial

    def record_emg(self):
        # clear previous data
        self.emg_data_queue.clear()

        # wait for the queue to fill (read in serial data)
        while len(self.emg_data_queue) < self.num_samps:

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

            self.emg_data_queue.append((datetime.datetime.now(), emgsample))

        return self.emg_data_queue




def record_training_data(recorder, num_repeats=1, save_fname='emgdata.pkl'):
    """
    Helper function to loop through all the grasps and record EMG
    """
    all_data = []

    # loop over postures and record data when user is ready
    print('Perform the indicated grasp:')
    for repeat in range(num_repeats):
        for posture in postures:
            print(f'\n{finger_groups[0]}: {posture[1]},  {finger_groups[1]}: {posture[2]}')
            
            if show_gesture_image:
                showGestureImage(posture[0], f'Get ready...')
                time.sleep(0.1)
                showGestureImage(posture[0], f'RECORDING')
            else:
                input('Press enter when ready...')
            data = recorder.record_emg()
            all_data.append((posture[0], data.copy()))
            print('done')

    # # debug
    # for d in all_data:
    #     print(d[0])
    #     print(d[1][1])

    # save data
    with open(save_fname, 'wb') as handle:
        pickle.dump(all_data, handle, protocol=pickle.HIGHEST_PROTOCOL)
    print('\nSaved')




if __name__ == "__main__":
    btserial = connectBTSerial()
    recorder = EmgRecorder(btserial, num_samps=SAMPLING_RATE*seconds_per_grasp)
    record_training_data(recorder, num_repeats=num_repeats, save_fname=out_fname)
