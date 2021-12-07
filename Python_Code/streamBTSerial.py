import sys
import glob
import serial
import time

# (see the main function at the bottom)


def get_serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
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
    print(f'You chose port {ports[value]}')

    ser = serial.Serial(ports[value], 115200)
    ser.flushInput()
    return ser


def readSerial(ser):
    '''
    Prints out received serial values. Each value should be on a newline (i.e. use Serial.println() on arduino)
    '''
    while True:
        ser_bytes = ser.readline()
        print(ser_bytes[0:len(ser_bytes)-2])
        # decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8")) # remove the last two bytes ('\n') and convert to float
        # print(decoded_bytes)


def readSerialEMG(ser):
    '''
    Prints out a received EMG sample containing data from 8 channels.
    For each time sample, the ESP32 should first send the startID (9999), followed by the 8 samples. The startID and
    each sample should be followed by a newline. (Serial.println(9999); Serial.println(electrode1); ...)
    '''

    NUM_ELCTRODES = 8

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






if __name__ == '__main__':

    ser = connectBTSerial()
    readSerial(ser)
    # readSerialEMG(ser)








