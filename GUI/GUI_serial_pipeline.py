import subprocess, sys, numpy
import csv
import itertools
from PyQt4 import QtGui
import serial, time
#from __future__ import division

from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QT as NavigationToolbar
import matplotlib.pyplot as plt

################ DEFINE global variable ###########
BUFFER_SIZE = 2
buffer_angle = []           # global variable
buffer_distance = []
filename = "sample_data_for_kacao.csv"

buffer_angle__1 = []        # temp var
buffer_distance_1 = []      # temp var
line = 0
port='COM22'
baudrate = '115200'
timeout = 0                 #time.time() + 30/120   Mike suggestion
sample_rate = 44100
##################################################


def __serial_config():
    print ("serial config ...")
    global port, baudrate, timeout
    try:
        ser = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
    except serial.SerialException:
        print ("Serial connection failed")


def __scan_microphone():
    with serial.Serial(port=port, baudrate=baudrate, timeout=timeout) as ser:
        try:
            row = ser.readline().rstrip()
            if row:
                print ("debugging __scan_microphone")
                print(row)
                x, y, trash, trash2 = row.split(',')
                print "x=", x
                #buffer_distance_1.append(float(y))
                #buffer_angle__1.append(float(x))
        except ValueError:
            x = 0
            y = 0

    ser.close()



class Window(QtGui.QDialog):
    # set up the frame for the software
    def __init__(self, parent=None):
        super(Window, self).__init__(parent)

        # a figure instance to plot on
        self.figure = plt.figure()
        self.canvas = FigureCanvas(self.figure)
        # it takes the Canvas widget and a parent
        self.toolbar = NavigationToolbar(self.canvas, self)
        # Just some button connected to `plot` method
        self.button = QtGui.QPushButton('Plot')
        self.button.clicked.connect(self.plot_csv)                      # Plot from csv file after Click the button
        #self.button.clicked.connect(self.plot)

        # set the layout
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        layout.addWidget(self.button)
        self.setLayout(layout)



    # average the array list
    def __average_list(self, numbers):
        try:
            z = sum(numbers) / float(len(numbers))
        except ZeroDivisionError:
            z = 0
        return z


    # grasp the distance + angle from csv file
    def __read_csv_file(self, name=''):
        global line, BUFFER_SIZE
        print "Before read csv Line = ", line
        del buffer_angle[:]
        del buffer_distance[:]
        loop_counter = 0
        try:
            with open(name, 'rb') as csvfile:
                reading = csv.reader(csvfile, dialect='excel', lineterminator='\n', delimiter=',')
                # next(reading)                               # skip the header
                for row in itertools.islice(reading, line, line + BUFFER_SIZE):
                    if (loop_counter > BUFFER_SIZE):
                        break;
                    try:                                      # insert number into the list + CONVERT data type
                        buffer_angle.append(float(row[1]))
                        buffer_distance.append(float(row[0]))
                        loop_counter += 1
                        line += 1
                    except ValueError:
                        continue
        finally:
            csvfile.close()

        print "After all =", line
        print "\n"


    # plot the data from the CSV file sample
    def plot_csv(self):
        global line
        print "plot", line, buffer_angle, buffer_distance
        self.__read_csv_file(filename)                              # value stored in buffer1, buffer2

        angle = self.__average_list(buffer_angle)
        angle = angle / 180.0 * 3.141593
        distance = self.__average_list(buffer_distance)

        ax = self.figure.add_axes([0.1, 0.1, 0.8, 0.8], polar=True)
        # ax = self.figure.add_subplot(111, polar = True)
        #ax.hold(False)                                              # discards the old graph
        ax.grid(True)
        ax.spines['polar'].set_visible(False)
        ax.set_theta_zero_location('N')                             # 0 degree in the north
        # ax.set_rlim(0,3.2)
        ax.set_ylim(0, 10.1)
        ax.set_yticks(numpy.arange(0, 12.0, 1.0))
        ax.scatter(angle, distance, c='r', s=100)                   # plot the first microphone

        self.canvas.draw()



    # Plot the polar graph.
    def plot(self):
        global buffer_distance_1, buffer_angle__1
        while True:
            self.__scan_microphone()
            angle = self.__average_list(buffer_angle__1)
            distance = self.__average_list(buffer_distance_1)
            angle = angle/ 180.0 * 3.141593                                 # convert deg --> rad

            ax = self.figure.add_axes([0.1, 0.1, 0.8, 0.8], polar=True)
            #ax = self.figure.add_subplot(111, polar = True)
            #ax.hold(False)                                                 # discards the old graph
            ax.grid(True)
            ax.spines['polar'].set_visible(False)
            ax.set_theta_zero_location('N')                                 # 0 degree in the north
            #ax.set_rlim(0,3.2)
            ax.set_ylim(0, 2.4)
            ax.set_yticks(numpy.arange(0, 2.4, 0.4))
            ax.scatter(angle, distance, c='r', s = 100)                     # plot the first microphone

            self.canvas.draw()


    # set up the serial connection
    def __serial_config(self):
        print ("serial config ...")
        global port, baudrate, timeout
        try:
            ser = serial.Serial(port=port, baudrate=baudrate, timeout=timeout)
        except serial.SerialException:
            print ("Serial connection failed")



    # grasp the live data from Serial connection
    def __scan_microphone(self):
        with serial.Serial(port=port, baudrate=baudrate, timeout=timeout) as ser:
            try:
                row = ser.readline().rstrip()
                # distance = line[0], angle = line[1]
                if row:
                    print ("debugging __scan_microphone")
                    print(row)
                    x, y, trash, trash2 = row.split(',')

                    buffer_distance_1.append(float(y))
                    buffer_angle__1.append(float(x))
            except ValueError:
                x = 0
                y = 0




# MAIN function
if __name__ == '__main__':
    __serial_config()                               # set up the serial port
    app = QtGui.QApplication(sys.argv)
    main = Window()
    main.show()
    sys.exit(app.exec_())



#Subprocess's Popen command with piped output and active shell
def Popen(cmd):
    return subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            shell=True).communicate()[0].rstrip()

#Subprocess's Popen command for use in an iterator
def PopenIter(cmd):
    return subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            shell=True).stdout.readline

