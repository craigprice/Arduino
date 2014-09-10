#!python

import time
import urllib2
import readTempArduino
import os
import struct
import subprocess
import sys
from datetime import datetime
import socket

secsBetweenFetches = 10

try:
    readTempArduino.ard.readInData()
except:
    readTempArduino.ard.writeError("Error trying to perform the first" +
    "readInData function that initializes collecting the Environment data" +
    "within the \"collectEnvironmentData\" class")
    print "Exception Raised Trying to Read in Data"
    print "Halting"
    raise

readTempArduino.ard.writeMetadata()

count = 0

while True:
    now = datetime.now()
    if now.hour == 23:
        if now.minute > 58:
            break
    count += 1
    
    try:
        readTempArduino.ard.readInData()
        readTempArduino.ard.writeMeasurement()
        readTempArduino.ard.checkForRails()
    except (urllib2.URLError, struct.error, socket.error):
        readTempArduino.ard.writeError("Error trying to read/write data within" +
        "the loop of the \"collectEnvironmentData\" script. Will skip this" +
        "moment's collection and try again.")
        print "Exception Raised Trying to Read/Write Data"
    except:
        readTempArduino.ard.writeError()
        print "Unknown Error: Halting"
        raise
    readTempArduino.ard.checkForRails()
    time.sleep(secsBetweenFetches)
    if count%10 == 0:
        subprocess.call([sys.executable, "C:\\EnvironmentData\\plotTempArduino.py"])
        count = 0