import re
import math

class datafile:
    def __init__(self,filename):
        self.filename = filename
        self.result = []
        self.ulCurTimeList = []
        self.TimeStampList = []
        self.DelayList = []
        
        self.TotalDataIn = 0
        self.TotalDataOut = 0
        self.TotalFrameOut = 0

        self.CurrentDelay = 0
        self.CurrentDataOut = 0
        self._analyze_total_status()
        self._analyze()

    def _analyze_total_status(self):
        with open(self.filename) as f:
            rex = "TotalInputData:\s+(\d+)\s"
            for line in f:
                matchResult = re.search(rex,line)
                if matchResult:
                    self.TotalDataIn = int(matchResult.groups()[0])
                            
    def _analyze(self):
        with open(self.filename) as f:       
            rex_textline = ("TimeStamp Delta:\s+(\d+)\s","ulCurTime Delta:\s+(\d+)\s","Timestamp and CurTime Delta:\s+(\d+)\s","DeliverDataInByte:\s+(\d+)\s")
            for line in f:
                for idx in range(4):
                    matchResult = re.search(rex_textline[idx],line)
                    if matchResult:                     
                        if idx==3:
                            self.CurrentDataOut = int(matchResult.groups()[0])
                            self.TotalDataOut = self.CurrentDataOut #get the last line
                        if idx==2:
                            self.DelayList.append(int(matchResult.groups()[0]))
                            self.TotalFrameOut += 1
                        if idx==1:
                            self.ulCurTimeList.append(int(matchResult.groups()[0]))
                        if idx==0:
                            self.TimeStampList.append(int(matchResult.groups()[0]))


      



