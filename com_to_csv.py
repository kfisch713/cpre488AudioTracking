#! /usr/bin/env python3
import serial
import csv
import sys
import wave
import struct

sample_rate = 44100
 

if __name__=='__main__':
    print ("Starting")
    try:
        ser = serial.Serial(port='COM10', baudrate=115200, timeout=1)

        print("connected to: " + ser.portstr)
        wave_file = wave.open("two_tones.wav", 'w')
        wave_file.setparams((1, 1, sample_rate, 0, 'NONE', 'not compressed'))
        print("created wav file")
        
        values = []
        sample_len = sample_rate * 5
        
        #f = open('output.csv', 'w')
        for i in range(1, sample_len):
            try:
                # Read a line and convert it from b'xxx\r\n' to xxx
                line = ser.readline().decode('utf-8').strip()
                line = int(line)
                #print(line)
                #adc_value, time_value = str(line).split(',')
                
                if line:  # If it isn't a blank line
                    #f.write(line + '\n')
                    packed_value = struct.pack('h', line)
                    values.append(packed_value)
            except UnicodeDecodeError:
                pass
            except ValueError:
                pass
            
            
        #f.close()
        print("out of loop")
        value_str = b''.join(values)
        wave_file.writeframes(value_str)
        wave_file.close()
        
    except serial.SerialException:
        print("failed to make connection")
    
