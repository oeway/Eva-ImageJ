#!/usr/bin/env python
"""\
Stream g-code to grbl controller

This script differs from the simple_stream.py script by 
tracking the number of characters in grbl's serial read
buffer. This allows grbl to fetch the next line directly
from the serial buffer and does not have to wait for a 
response from the computer. This effectively adds another
buffer layer to prevent buffer starvation.

TODO: - Add runtime command capabilities

Version: SKJ.20120110
"""
from serial_manager import SerialManager
import re
import time
import sys
import argparse
# import threading

RX_BUFFER_SIZE = 128

# Define command line argument interface
parser = argparse.ArgumentParser(description='Stream g-code file to grbl. (pySerial and argparse libraries required)')
parser.add_argument('gcode_file', type=argparse.FileType('r'),
        help='g-code filename to be streamed')
parser.add_argument('device_file',
        help='serial device path')
parser.add_argument('-q','--quiet',action='store_true', default=False, 
        help='suppress output text')
args = parser.parse_args()

SerialManager.connect(args.device_file, 9600)
s = SerialManager
# Periodic timer to query for status reports
# TODO: Need to track down why this doesn't restart consistently before a release.
# def periodic():
#     s.write('?')
#     t = threading.Timer(0.1, periodic) # In seconds
#     t.start()

# Initialize

f = args.gcode_file
verbose = True
if args.quiet : verbose = False

# Wake up grbl
print "Initializing grbl..."
s.write("\r\n\r\n")


# Stream g-code to grbl
print "Streaming ", args.gcode_file.name, " to ", args.device_file
l_count = 0
g_count = 0
c_line = []
# periodic() # Start status report periodic timer
for line in f:
    if '(' in line:
        continue
    l_count += 1 # Iterate line counter
#     l_block = re.sub('\s|\(.*?\)','',line).upper() # Strip comments/spaces/new line and capitalize
    l_block = line.strip()
    c_line.append(len(l_block)+1) # Track number of characters in grbl serial read buffer
    grbl_out = '' 
    ret = ''
    if verbose: print "SND: " + str(l_count) + " : " + l_block,
    s.write(l_block + '\n') # Send block to grbl
    if verbose : print "BUF:",str(sum(c_line)),"REC:",grbl_out
    while sum(c_line) >= RX_BUFFER_SIZE-1 :
        ret = s.read_to('\n')
        out_temp = ret.strip() # Wait for grbl response
        if out_temp.find('ok') < 0 and out_temp.find('error') < 0 :
            print "  Debug: ",out_temp # Debug response
        else :
            grbl_out += out_temp;
            g_count += 1 # Iterate g-code counter
            grbl_out += str(g_count); # Add line finished indicator
            del c_line[0]


# Wait for user input after streaming is completed
print "G-code streaming finished!\n"
print "WARNING: Wait until grbl completes buffered g-code blocks before exiting."
raw_input("  Press <Enter> to exit and disable grbl.") 

# Close file and serial port
f.close()
s.close()