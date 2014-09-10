#!python

import urllib2
import time
import struct
import numpy as np
import datetime
import matplotlib.pyplot as plt
import readTempArduino
from matplotlib.dates import HourLocator, MinuteLocator, DateFormatter
import XTSMobjectify
import traceback
import sys
import xml
import math
import gnosis.xml.objectify

fileName = readTempArduino.ard.fileName
startHour = False
startMinute = False
endHour = False
endMinute = False
if len(sys.argv) > 1:
    fileName = sys.argv[1]
    if fileName == 'today':
        fileName = readTempArduino.ard.fileName
if len(sys.argv) > 3:
    startHour = int(sys.argv[2])
    startMinute = int(sys.argv[3])
if len(sys.argv) > 4:
    startHour = int(sys.argv[2])
    startMinute = int(sys.argv[3])
    endHour = int(sys.argv[4])
if len(sys.argv) > 5:
    startHour = int(sys.argv[2])
    startMinute = int(sys.argv[3])
    endHour = int(sys.argv[4])
    endMinute = int(sys.argv[5])

print "Plotting Data File:\n" + fileName

data = "<Temperature_Data>\n"
with open(fileName,'r') as f:
    for line in f:
        data = data + line
    f.close()
data = data + "</Temperature_Data>"

try:
    Temperature_Data = gnosis.xml.objectify.make_instance(data)
except xml.parsers.expat.ExpatError as e:
    readTempArduino.ard.writeError("Error in trying to make the data into an xml object within the \"plotTempArduino\" script.")
    print sys.exc_info()
    print traceback.format_exc()
    sys.exit()
    
measurements = Temperature_Data.Measurement

fig = plt.figure(figsize=(15,9))
reservoirData = []
nufernData = []
coldCityWaterFlowData = []
reservoirLoopFlowData = []
timeXaxis = []

res0Conversion = 0
res0Vset = 0
nuf0Conversion = 0
nuf0Vset = 0

if isinstance(Temperature_Data.Metadata, list) is True:
    metadata = Temperature_Data.Metadata[0]
else:
    metadata = Temperature_Data.Metadata


for chan in metadata.Channel:
    if chan.Description.PCDATA == "Res0":
        res0Conversion = float(chan.ErrorVoltageTomKConversion.PCDATA)
        res0Vset = int(chan.VSet.PCDATA)
    if chan.Description.PCDATA == "Nuf0":
        nuf0Conversion = float(chan.ErrorVoltageTomKConversion.PCDATA)
        nuf0Vset = int(chan.VSet.PCDATA)

for m in measurements:
    if (hasattr(m, "Timestamp") and hasattr(m, "Channel")):
        year = int(m.Timestamp.Year.PCDATA)
        month = int(m.Timestamp.Month.PCDATA)
        day = int(m.Timestamp.Day.PCDATA)
        hour = int(m.Timestamp.Hour.PCDATA)
        minute = int(m.Timestamp.Minute.PCDATA)
        second = int(m.Timestamp.Second.PCDATA)
        if startHour != False:
            if hour < startHour:
                continue
            if ((endHour != False)and(hour > endHour)):
                continue
            if ((hour == startHour) and (minute < startMinute)):
                continue
            if ((endHour != False) and (hour == endHour) and (minute > endMinute)):
                continue
        timeXaxis.append(datetime.datetime(year,month,day,hour,minute,second))
        #flow
        if hasattr(m, "AvePulsesPerSecond_CCW") and  hasattr(metadata, "PulsesPerGallon_CCW") :
            temp = 60*float(m.AvePulsesPerSecond_CCW.PCDATA)
            temp = temp / float(metadata.PulsesPerGallon_CCW.PCDATA)
            if math.isnan(temp) == True:
                temp = 0
            coldCityWaterFlowData.append(temp)
            temp = 60*float(m.AvePulsesPerSecond_RL.PCDATA)
            temp = temp / float(metadata.PulsesPerGallon_RL.PCDATA)
            if math.isnan(temp) == True:
                temp = 0
            reservoirLoopFlowData.append(temp)
        else:
            coldCityWaterFlowData.append(0)
            reservoirLoopFlowData.append(0)
            
        for chan in m.Channel:
            if chan.Description.PCDATA== "Res0":
                mKAround21C = res0Conversion * (res0Vset - (1.0*int(chan.ErrorVoltage.PCDATA)))
                reservoirData.append(mKAround21C)
            if chan.Description.PCDATA == "Nuf0":
                mKAround24C = nuf0Conversion * (nuf0Vset - (1.0*int(chan.ErrorVoltage.PCDATA)))
                nufernData.append(mKAround24C)
            

#plotting Reservoir data
ax = plt.subplot(411)
ax.xaxis.set_major_locator(HourLocator())
ax.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
#ax.xaxis.set_minor_locator(MinuteLocator())
plt.tick_params(\
    axis='x',          # changes apply to the x-axis
    which='both',      # both major and minor ticks are affected
    bottom='off',      # ticks along the bottom edge are off
    top='off',         # ticks along the top edge are off
    labelbottom='off')
    
plt.ylabel("Resrvoir Temperature\n deviation [mK]")
line, = ax.plot_date(timeXaxis, reservoirData, 'r-')    

ax = plt.subplot(412)
ax.xaxis.set_major_locator(HourLocator())
ax.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
plt.ylabel("Cold City Water Flow\n Rate [GPM]")
line, = ax.plot_date(timeXaxis, coldCityWaterFlowData, 'r-')

ax = plt.subplot(413)
ax.xaxis.set_major_locator(HourLocator())
ax.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
plt.ylabel("Reservoir Loop Flow\n Rate [GPM]")
line, = ax.plot_date(timeXaxis, reservoirLoopFlowData, 'r-')

#plotting nufern data
ax=plt.subplot(414)
ax.xaxis.set_major_locator(HourLocator())
ax.xaxis.set_major_formatter(DateFormatter('%H:%M:%S'))
plt.ylabel("Nufern Temperature\n deviation [mK]")
plt.xlabel("Time [Hour:Minute:Second]")
line1, = ax.plot_date(timeXaxis, nufernData, 'r-')

plt.xticks(rotation=25)

ax.autoscale_view()

#plt.draw()
plt.savefig("C:\\wamp\\www\\environmentData.svg")

print sys.argv
if len(sys.argv) < 3:
    plt.savefig(fileName[:-4] + ".svg")

print "done"