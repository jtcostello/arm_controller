from collections import deque
from threading import Lock
import pickle
import myo
import time
from pathlib import Path


out_fname = 'emgdata08.pkl'     # the filename of where to save the recorded data
seconds_per_grasp = 4           # how many seconds of data to record per grasp
num_repeats = 4                 # how many times to cycle through all grasps



sdk_path = Path(__file__).parent / 'myo-sdk-win-0.9.0'

class EmgRecorder(myo.DeviceListener):
    """
    Records EMG with the specified number of samples
    """
    def __init__(self, num_samps=200):
        self.lock = Lock()
        self.num_samps = num_samps
        self.emg_data_queue = deque(maxlen=num_samps+1)
        self.done_recording = False

    def on_connected(self, event):
        event.device.stream_emg(True)

    def on_emg(self, event):
        with self.lock:
            if not self.done_recording:
                self.emg_data_queue.append((event.timestamp, event.emg))
                if len(self.emg_data_queue) >= self.num_samps:
                    self.done_recording = True

    def record_emg(self):
        # clear previous data
        with self.lock:
            self.emg_data_queue.clear()
            self.done_recording = False
        # wait for the queue to fill (not an ideal way to do this, but it works. ideally would use threading events)
        done = False
        while not done:
            with self.lock:
                done = self.done_recording
            time.sleep(0.01)
        with self.lock:
            return self.emg_data_queue


def record_training_data(recorder, num_repeats=1, save_fname='emgdata.pkl'):
    """
    Helper function to loop through all the grasps and record EMG
    """
    finger_groups = ['pinch', 'MRP']
    postures = [[0, 'neutral', 'neutral'], [1, 'open', 'open'], [2, 'close', 'open'], [3, 'close', 'close'], [4, 'open', 'close']]
    all_data = []

    # loop over postures and record data when user is ready
    print('Perform the indicated grasp:')
    for repeat in range(num_repeats):
        for posture in postures:
            print(f'\n{finger_groups[0]}: {posture[1]},  {finger_groups[1]}: {posture[2]}')
            input('Press enter when ready...')
            data = recorder.record_emg()
            all_data.append((posture[0], data.copy()))
            # print('data length = ' + str(len(data)))
            print('done')
            # print(data[1])
            # print(all_data[-1][1][1])

    # # debug
    # for d in all_data:
    #     print(d[0])
    #     print(d[1][1])

    # save data
    with open(save_fname, 'wb') as handle:
        pickle.dump(all_data, handle, protocol=pickle.HIGHEST_PROTOCOL)
    print('\nSaved')


if __name__ == "__main__":
    myo.init(sdk_path=sdk_path)
    hub = myo.Hub()
    recorder = EmgRecorder(num_samps=200*seconds_per_grasp)
    with hub.run_in_background(recorder.on_event):
        record_training_data(recorder, num_repeats=num_repeats, save_fname=out_fname)
