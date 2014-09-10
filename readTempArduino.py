#!python

import urllib2
import time
import datetime
import struct
import numpy as np
import sys
import traceback

import subprocess
import winsound

#dynamically load the XTSM class on Vortex
#import requests
#r = requests.get('http://coldatomfqh.phys.psu.edu:8081/MetaViewer/XTSMobjectify.py')
'''
import imp
xtsmObjectify = imp.new_module("xtsmObjectify")
xtsmObjectify_code = r.text
exec xtsmObjectify_code
xtsmObjectify.XTSM_core.__module__


import imp
foo = imp.new_module("foo")
foo_code = str(r.text)
exec foo_code in foo.__dict__
foo.XTSM_core.__module__
'''

from XTSMobjectify import XTSM_core

ipaddress = "http://10.1.1.200"
#vset_reservoir = 4100 #The conversion to mK (centered at 21C) is approx, 3.5 * (vset_reservoir - err_voltage)
#vset_nufern = 2710#2970  The conversion to mK (centered at 24C) is approx, 2.5 * (vset_nufern - err_voltage)

class Second(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Minute(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class Hour(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Day(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Month(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Year(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Timestamp(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        date = datetime.datetime.now()
        second = Second(value=str(date.second))
        self.insert(second)
        minute = Minute(value=str(date.minute))
        self.insert(minute)
        hour = Hour(value=str(date.hour))
        self.insert(hour)
        day = Day(value=str(date.day))
        self.insert(day)
        month = Month(value=str(date.month))
        self.insert(month)
        year = Year(value=str(date.year))
        self.insert(year)
            
                
class MacAddress(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class MaxDelay(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class MicrosecondDelayFor220Phase(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class TotalNumFlowAverages (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class PulsesPerGallon_CCW(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class LoopsOpen_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class LoopsClosed_CCW(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class SwitchOpen_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class PulsesPerSecond_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class NumFlowAverages_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class SumPulsesPerSecond_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        XTSM_core.__init__(self,value)
                
class AvePulsesPerSecond_CCW (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class PulsesPerGallon_RL(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class LoopsOpen_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class LoopsClosed_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class SwitchOpen_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class PulsesPerSecond_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class NumFlowAverages_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class SumPulsesPerSecond_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class AvePulsesPerSecond_RL (XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class DIOOutChannel(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class Enable(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class RSlope(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class VSet(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class HDelay(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class LowpassTimeConstant(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class ProportionalOscillationAmplitude(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class ProportionalOscillationTime(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class OverallScalingGain(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class IntegralTime(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class DerivativeTime(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class AntiWindup(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class IntegralCoefficient(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class IntegralGain(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class DerivativeCoefficient(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class DerivativeGain(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class ErrorVoltageTomKConversion(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class Description(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class ProportionalResponse(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class IntegralResponse(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class DerivativeResponse(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
                
class CombinedResponseSaturated(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        
class Outphase(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class ChannelDescription(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class ErrorVoltage(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Channel(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Metadata(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)

class Measurement(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        self.insert(Timestamp())
        
class ProgramError(XTSM_core):
    def __init__(self,value=None):
        XTSM_core.__init__(self,value)
        self.insert(Timestamp())
        
def nextBinaryData(tcp_response, bytesToRead, typeToUnpack):
    #tcp_response is what is returned from urllib2.urlopen()
    #bytesToRead is the number of bytes in the next data element to be parsed
    #typeToUnpack is a character string like '!h' that tells python how to understand the bits
    #cls is the xml class that the data falls in.
    #Returns the data value as a string
    try:
        binaryData = tcp_response.read(bytesToRead)
    except struct.error:
        ard.writeError("Error reading the next part of the binary data blob in function \"nextBinaryData\".")
        raise
    value = struct.unpack(typeToUnpack, binaryData)[0]
    return str(value)
    
def nextBinaryData_ToXML(tcp_response, bytesToRead, typeToUnpack, cls):
    #cls is the xml class that the data falls in
    #Returns the data value as a xml class
    value = nextBinaryData(tcp_response, bytesToRead, typeToUnpack)
    return cls(value)        
        
class Arduino():
    def __init__(self):
        self.ipaddress = ipaddress
        #self.vset_reservoir = vset_reservoir #The conversion to mK (centered at 21C) is approx, 3.5 * (vset_reservoir - err_voltage)
        #self.vset_nufern = vset_nufern#2970  The conversion to mK (centered at 24C) is approx, 2.5 * (vset_nufern - err_voltage)
        self.dateArduinoInit = datetime.datetime.now()
        dateStr = "%s_%s_%s" % (self.dateArduinoInit.year, self.dateArduinoInit.month, self.dateArduinoInit.day)
        self.fileName = "C:\\EnvironmentData\\readTempArduino\\" + str(dateStr)+"_TemperatureServoData.txt"
        self.fileNameBackup = "C:\\EnvironmentData\\readTempArduino\\Backup_" + str(dateStr)+"_TemperatureServoData.txt"
        self.arduinoRequest = urllib2.Request(ipaddress)
        self.measurement = None
        self.metadata = None
        
    def readInData(self):
        response = None
        try:
            response = urllib2.urlopen(self.arduinoRequest)
        except urllib2.URLError:
            self.writeError("Error in opening the connection to the arduino in function \"readInData\".")
            raise
        httpstring = ""
        for i in range(0,96):#Reading in the HTTP header. After loop, bytes are binary data
            httpstring = httpstring + nextBinaryData(response, 1, 'c')
        meas = Measurement()    
        meta = Metadata()
        value = []
        
        #Fetch the MacAddress
        # 'B' cooresponds to uint8_t.
        # !' is big-endian (network)
        for i in range(0,6):
            value_str = nextBinaryData(response, 1, '!B')
            value.append(hex(int(value_str)))
        meta.insert(MacAddress(str(value)),pos=sys.maxint)
        # 'h' cooresponds to int16_t 
        meta.insert(nextBinaryData_ToXML(response, 2, '!h', MaxDelay),pos=sys.maxint)
        meta.insert(nextBinaryData_ToXML(response, 2, '!h', MicrosecondDelayFor220Phase),pos=sys.maxint)
        
        #Flow Switches
        meta.insert(nextBinaryData_ToXML(response, 2, '!h', TotalNumFlowAverages),pos=sys.maxint)
        meta.insert(nextBinaryData_ToXML(response, 1, '!B', PulsesPerGallon_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', LoopsOpen_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', LoopsClosed_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 1, '?',  SwitchOpen_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', PulsesPerSecond_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', NumFlowAverages_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', SumPulsesPerSecond_CCW),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', AvePulsesPerSecond_CCW),pos=sys.maxint)
        meta.insert(nextBinaryData_ToXML(response, 1, '!B', PulsesPerGallon_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', LoopsOpen_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', LoopsClosed_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 1, '?',  SwitchOpen_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', PulsesPerSecond_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 2, '!h', NumFlowAverages_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', SumPulsesPerSecond_RL),pos=sys.maxint)
        meas.insert(nextBinaryData_ToXML(response, 4, '!f', AvePulsesPerSecond_RL),pos=sys.maxint)
        #End FLow Switches
        
        #Fetching Per Channel
        for i in range(0,8):
            chan = Channel()
            #Fetching DIOOutChannel
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', DIOOutChannel),pos=sys.maxint)
            #Fetching Enable
            #'?' cooresponds to boolean
            #enabled when "LOW_MCP (= 0)" which is normally "false". "Not"ed so that LOW is (enabled) "true"
            value = bool("True" == nextBinaryData(response, 1, '?'))
            chan.insert(Enable(str(not(value))),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', RSlope),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', VSet),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', HDelay),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', LowpassTimeConstant),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', ProportionalOscillationAmplitude),pos=sys.maxint)
            #'f' cooresponds to floats
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', ProportionalOscillationTime),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', OverallScalingGain),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', IntegralTime),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', DerivativeTime),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', AntiWindup),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', IntegralCoefficient),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', IntegralGain),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', DerivativeCoefficient),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', DerivativeGain),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', ErrorVoltageTomKConversion),pos=sys.maxint)
            #Fetching Description
            value = ''
            for j in range(0,4):
                value_str = nextBinaryData(response, 1, 'c')
                value = value + value_str
            chan.insert(Description(str(value)),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', ProportionalResponse),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', IntegralResponse),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', DerivativeResponse),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 4, '!f', CombinedResponseSaturated),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', Outphase),pos=sys.maxint)
            chan.insert(nextBinaryData_ToXML(response, 2, '!h', ErrorVoltage),pos=sys.maxint)
            
            measurementsOfChannel = Channel()
            measurementsOfChannel.addAttribute("ChannelNumber",str(i))
            measurementsOfChannel.insert(chan.Description,pos=sys.maxint)
            measurementsOfChannel.insert(chan.Enable,pos=sys.maxint)
            measurementsOfChannel.insert(chan.ProportionalResponse,pos=sys.maxint)
            measurementsOfChannel.insert(chan.IntegralResponse,pos=sys.maxint)
            measurementsOfChannel.insert(chan.DerivativeResponse,pos=sys.maxint)
            measurementsOfChannel.insert(chan.CombinedResponseSaturated,pos=sys.maxint)
            measurementsOfChannel.insert(chan.Outphase,pos=sys.maxint)
            measurementsOfChannel.insert(chan.ErrorVoltage,pos=sys.maxint)
            
            metadataOfChannel = Channel()
            metadataOfChannel.addAttribute("ChannelNumber",str(i))
            metadataOfChannel.insert(chan.Description,pos=sys.maxint)
            metadataOfChannel.insert(chan.DIOOutChannel,pos=sys.maxint)
            metadataOfChannel.insert(chan.Enable,pos=sys.maxint)
            metadataOfChannel.insert(chan.RSlope,pos=sys.maxint)
            metadataOfChannel.insert(chan.VSet,pos=sys.maxint)
            metadataOfChannel.insert(chan.HDelay,pos=sys.maxint)
            metadataOfChannel.insert(chan.LowpassTimeConstant,pos=sys.maxint)
            metadataOfChannel.insert(chan.ProportionalOscillationAmplitude,pos=sys.maxint)
            metadataOfChannel.insert(chan.ProportionalOscillationTime,pos=sys.maxint)
            metadataOfChannel.insert(chan.OverallScalingGain,pos=sys.maxint)
            metadataOfChannel.insert(chan.IntegralTime,pos=sys.maxint)
            metadataOfChannel.insert(chan.DerivativeTime,pos=sys.maxint)
            metadataOfChannel.insert(chan.AntiWindup,pos=sys.maxint)
            metadataOfChannel.insert(chan.IntegralCoefficient,pos=sys.maxint)
            metadataOfChannel.insert(chan.IntegralGain,pos=sys.maxint)
            metadataOfChannel.insert(chan.DerivativeCoefficient,pos=sys.maxint)
            metadataOfChannel.insert(chan.DerivativeGain,pos=sys.maxint)
            metadataOfChannel.insert(chan.ErrorVoltageTomKConversion,pos=sys.maxint)
            print chan.Description.PCDATA
            #This if statement is also implicitly checking to make sure the 
            #Binary data was not corrupted.
            if (chan.Description.PCDATA == "Nuf0") or (chan.Description.PCDATA == "Res0"):
                meas.insert(measurementsOfChannel,pos=sys.maxint)
                meta.insert(metadataOfChannel,pos=sys.maxint)
        
        self.metadata = meta
        self.measurement = meas
        
        
    def checkForRails(self):
        nuf0Conversion = 0
        nuf0Vset = 0
        nufTempmK = 0
        try:
            metaChan = self.metadata.Channel
        except AttributeError:
            errMessage = ("Within class readTempArduino, function" +
            " checkForRails().\nAttribute Error from " +
            "'metaChan = self.metadata.Channel'.\nLikely due to a connection" +
            "failure and a corruption of the metadata in the connection.\n" +
            "Skipping the rest of the check.")
            print errMessage
            ard.writeError(errMessage)
            return
        for chan in self.metadata.Channel:
            if chan.Description.PCDATA == "Nuf0":
                    nuf0Conversion = float(chan.ErrorVoltageTomKConversion.PCDATA)
                    nuf0Vset = int(chan.VSet.PCDATA)
        for chan in self.measurement.Channel:
            if chan.Description.PCDATA == "Nuf0":
                nufTempmK = nuf0Conversion * (nuf0Vset - (1.0*int(chan.ErrorVoltage.PCDATA)))
        print "Nufern Temperature away from set point [mK]:\n",nufTempmK
        if nufTempmK > 1000 or nufTempmK < -4000:
            winsound.Beep(2500,1000)
            message = "Check Nufern loop temperature"
            subprocess.call([sys.executable, "C:\\wamp\\www\\textToSpeech.py", message])

    def writeMeasurement(self):
        #Write to main file
        f = open(self.fileName,'a')
        f.write(self.measurement.write_xml())
        f.close()
        
        #Write to backup file
        f = open(self.fileNameBackup,'a')
        f.write(self.measurement.write_xml())
        f.close()

    
    def writeMetadata(self):
        #Write to main file
        f = open(self.fileName,'a')
        f.write(self.metadata.write_xml())
        f.close()
        
        #Write to backup file
        f = open(self.fileNameBackup,'a')
        f.write(self.metadata.write_xml())
        f.close()
        
    def writeError(self,annotation=""):
        errorString = ""
        errorList = traceback.format_exception(sys.exc_info()[0],sys.exc_info()[1],sys.exc_info()[2])
        errorListStrip = []
        for e in errorList:
            eStrip = str(e).translate(None, '<>')
            errorListStrip.append(eStrip)
        if annotation != "":
            errorListStrip.append(annotation)
        errorString = '\n'.join('{}: {}'.format(*k) for k in enumerate(errorListStrip))
        err = ProgramError(errorString)
        
        #Write to main file
        f = open(self.fileName,'a')
        f.write(err.write_xml())
        f.close()
        
        #Write to backup file
        f = open(self.fileNameBackup,'a')
        f.write(err.write_xml())
        f.close()
    
ard = Arduino()
try:
    ard.readInData()
except urllib2.URLError:
    ard.writeError("Error in performing the first readInData function within the \"Arduino\" class.")
    print "No Connection"

info = "Data Taken from the Arduino at IP address: " + ipaddress + "\n"
info += "This data is taken from the Octal Linearized Bridge.\n"
info += "The values are 13bit unsigned ints ranging from 0 to 8191.\n"
info += "The conversion from the error Voltage to mK is only decent for "
info += "temperatures within about a degree."
info += "The thermistor response is nonlinear in general. See The database for"
info += "detailed information on the resistance => temperature for larger voltages.\n"


ard.metadata = Metadata(info)
