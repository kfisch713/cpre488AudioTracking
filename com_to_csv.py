#! /usr/bin/env python3
import serial
import csv
import sys
#excel stuff
#from time import gmtime, strftime
#resultFile=open('MyData.csv','wb')
#end excel stuff
 

if __name__=='__main__':
    print ("Starting")
    try:
        ser = serial.Serial(port='COM19', baudrate=115200, timeout=1)
        print("connected to: " + ser.portstr)
        
        f = open('output.csv', 'w')
        while True:
            try:
                # Read a line and convert it from b'xxx\r\n' to xxx
                line = ser.readline().decode('utf-8').strip()
                print(line)
                adc_value, time_value = str(line).split(',')
                
                if line:  # If it isn't a blank line
                    f.write(line + '\n')
            except UnicodeDecodeError:
                pass
            except ValueError:
                pass
            
            
        f.close()
    except serial.SerialException:
        print("failed to make connection")
    
