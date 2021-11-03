# The MIT License (MIT)
#
# Copyright (c) 2017 Niklas Rosenstein
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

from matplotlib import pyplot as plt
from collections import deque
from threading import Lock, Thread

import myo
import numpy as np

sdk_path = 'C:/Users/Joey/Dropbox (University of Michigan)/EECS 473/Project/myo-sdk-win-0.9.0'


def moving_average(x, N):
    return np.convolve(x, np.ones(N)/N, mode='same')

def filt_emg(data_in):
    # data_in is a list with each entry (timestamp, emglist(8))
    emg_data = np.array([x[1] for x in data_in]).T # 1000x8
    data_filt = np.array([moving_average(x, 1) for x in emg_data])
    return data_filt



class EmgCollector(myo.DeviceListener):
  """
  Collects EMG data in a queue with *n* maximum number of elements.
  """

  def __init__(self, n):
    self.n = n
    self.lock = Lock()
    self.emg_data_queue = deque(maxlen=n)
    self.imu_data_queue = deque(maxlen=n)

  def get_emg_data(self):
    with self.lock:
      return list(self.emg_data_queue)

  def get_imu_data(self):
    with self.lock:
      return list(self.imu_data_queue)

  def on_connected(self, event):
    event.device.stream_emg(True)

  def on_emg(self, event):
    with self.lock:
      emg = event.emg
      emgfilt = [abs(x) for x in emg]
      self.emg_data_queue.append((event.timestamp, emgfilt))

  def on_orientation(self, event):
    with self.lock:
      self.imu_data_queue.append((event.timestamp, event.orientation))





class Plot(object):

  def __init__(self, listener):
    self.n = listener.n
    self.listener = listener
    self.fig = plt.figure()
    self.axes = [self.fig.add_subplot('81' + str(i)) for i in range(1, 9)]
    [(ax.set_ylim([-100, 100])) for ax in self.axes]
    self.graphs = [ax.plot(np.arange(self.n), np.zeros(self.n))[0] for ax in self.axes]
    plt.ion()

  def update_plot(self):
    emg_data = self.listener.get_emg_data()
    imu_data = self.listener.get_imu_data()

    # emg_data = np.array([x[1] for x in emg_data]).T
    emg_data = filt_emg(emg_data)

    for g, data in zip(self.graphs, emg_data):
      if len(data) < self.n:
        # Fill the left side with zeroes.
        data = np.concatenate([np.zeros(self.n - len(data)), data])
      g.set_ydata(data)
    plt.draw()

  def main(self):
    while True:
      self.update_plot()
      plt.pause(1.0 / 30)




def main():
  myo.init(sdk_path=sdk_path)
  hub = myo.Hub()
  listener = EmgCollector(800)
  with hub.run_in_background(listener.on_event):
    Plot(listener).main()


if __name__ == '__main__':
  main()