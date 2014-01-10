
import os
import re
import logAnalyze

class LogSummary:
    def __init__(self,DIR,RC):
        self.DIR = DIR
        self.RC = RC
        self.result = dict()        
        self._analyze()
        
    def _getQopAndQP(self,roots):
        if (self.RC):
            rex = "GOP_(\d)\\\\BR_(\d+)"
        else:
            rex = "GOP_(\d)\\\\QP_(\d+)"         
        matchResult = re.search(rex,roots)
        gop = '0'
        qp = '0'
        if matchResult:
            gop,qp = matchResult.groups()
        return (gop,qp)
    
    def _analyze(self):
        for roots,dirs,files in os.walk(self.DIR):
            if dirs == []:
                gop,qp = self._getQopAndQP(roots)
                #if gop not in self.result.keys():   self.result[gop] = dict()
                #if qp not in self.result[gop].keys():   self.result[gop][qp] = dict()
                    
                for dfile in files:
                    ext = os.path.splitext(dfile)[1:]
                    print (ext)
                    if ext != ('.log',):
                        continue
                    print (2,roots, dfile)
                    onefile = logAnalyze.datafile(os.path.join(roots,dfile))
                    if dfile not in self.result.keys() :self.result[dfile] = dict()
                    if gop not in self.result[dfile].keys() : self.result[dfile][gop] = dict()
                    if qp not in self.result[dfile][gop].keys() : self.result[dfile][gop][qp] = onefile

                                                      
    def outputOverallResult(self,outputTar = os.sys.stdout,anchor_gop="1"):
        print("{seq:35}\t{qp:3}\t{gop:3}\t{TotalDataIn:35}\t{TotalDataOut:35}\t{DataRatio:35}\t{TotalFrameOut:35}\t{ValidDelayFrame:35}\t{AverageDelay:35}\t{MaxDelay:35}\t".format(seq = "sequence",qp = "qp",gop = "gop",TotalDataIn = "TotalDataIn(B)",TotalDataOut = "TotalDataOut(B)", DataRatio = "DataRatio",TotalFrameOut = "TotalFrameOut",ValidDelayFrame = "ValidDelayFrame",AverageDelay = "AverageDelay(ms)",MaxDelay = "MaxDelay(ms)"),file = outputTar)

        TotalDataInList = []
        AverageDelayList = []
        MaxDelayList = []
        DataRatioList = []

        AverageDelayListAtFrame = []
        MaxDelayListAtFrame = []
        
        for dfile in self.result.keys():
            filename = dfile
            for gop in sorted(self.result[dfile].keys()):
                for qp in self.result[dfile][gop].keys():
                    print(" %-35s\t%3s\t" %(filename[:-16],str(qp)),end = "",file = outputTar)
                    filename = ""
                    ValidDelayList = []
                    ValidDelayListAtFrame = []
                    
                    TotalDataIn = self.result[dfile][gop][qp].TotalDataIn
                    TotalDataOut = self.result[dfile][gop][qp].TotalDataOut
                    TotalFrameOut = self.result[dfile][gop][qp].TotalFrameOut

                    ListLen = len(self.result[dfile][gop][qp].TimeStampList)
                    LastInputTime = self.result[dfile][gop][qp].TimeStampList[ListLen-1]

                    #print("TimeStampList= %d\n" %(len(self.result[dfile][gop][qp].TimeStampList)))
                    #print("ulCurTimeList %-d\n" %(len(self.result[dfile][gop][qp].ulCurTimeList)))
                    #print("DelayList %-d\n" %(len(self.result[dfile][gop][qp].DelayList)))
                    #print("TotalFrameOut %-d\n" %((self.result[dfile][gop][qp].TotalFrameOut)))
                    LastFinishedFrame = -1
                    for i in range(ListLen):
                        CurOutputTime = self.result[dfile][gop][qp].ulCurTimeList[i]

                        if i>0:
                            LastFinishedFrame = self.result[dfile][gop][qp].TimeStampList[i-1]
                        
                        if (CurOutputTime < LastInputTime+1000):
                            #print(" %d \t %d \t %d\n" %(i,CurOutputTime,LastInputTime))
                            ValidDelayList.append(self.result[dfile][gop][qp].DelayList[i])
                            if LastFinishedFrame!=-1:
                                ValidDelayListAtFrame.append(abs(CurOutputTime-LastFinishedFrame))
                            
                    
                    AverageDelay = sum(ValidDelayList)/len(ValidDelayList)
                    MaxDelay = max(ValidDelayList)


                    if len(ValidDelayListAtFrame)>0:
                        AverageDelayAtFrame = sum(ValidDelayListAtFrame)/len(ValidDelayListAtFrame)
                        MaxDelayAtFrame = max(ValidDelayListAtFrame)
                    else:
                        AverageDelayAtFrame = 0
                        MaxDelayAtFrame = 0
                    
                    TotalDataInList.append(TotalDataIn)
                    AverageDelayList.append(AverageDelay)
                    MaxDelayList.append(MaxDelay)

                    AverageDelayListAtFrame.append(AverageDelayAtFrame)
                    MaxDelayListAtFrame.append(MaxDelayAtFrame)
                    
                    DataRatioList.append(TotalDataOut/TotalDataIn)

                    print("%3d\t" %int(gop) + "%-5d\t" %TotalDataIn + "%-5d\t" %TotalDataOut + "%-10.3f\t" %(TotalDataOut/TotalDataIn) + "%-5d\t" %TotalFrameOut + "%-5d\t" %(len(ValidDelayList)) + "%6d\t" %AverageDelay + "%6d\t" %MaxDelay + " %6d\t " %AverageDelayAtFrame + " %6d\t " %MaxDelayAtFrame , end = "\n",file = outputTar)

        overall_TotalDataIn = sum(TotalDataInList)/len(TotalDataInList) 
        overall_AverageDelay = sum(AverageDelayList)/len(AverageDelayList)         
        overall_MaxDelay = max(MaxDelayList)
        overall_DataRatio = sum(DataRatioList)/len(DataRatioList) 

        overall_AverageDelayAtFrame = sum(AverageDelayListAtFrame)/len(AverageDelayListAtFrame)         
        overall_MaxDelayAtFrame = max(MaxDelayListAtFrame)
                                         
        print("over all state : overall_AverageDelay:\t%8.1f\toverall_MaxDelay \t%d\toverall_DataRatio \t%8.3f\toverall_averTotalDataIn\t%8.3f\tAverageDelayAtFrame \t%8.3f\tMaxDelayAtFrame\t%8.3f\t" %(overall_AverageDelay,overall_MaxDelay,overall_DataRatio,overall_TotalDataIn,overall_AverageDelayAtFrame,overall_MaxDelayAtFrame),file = outputTar)

                

def main():
   
    anchor_DIR = r".\_RC0_BGD1_LTR1_SCENE1_test_me16_3_me8_7_fmehash_0_ltrnum_4\file"
    print(anchor_DIR)
    gop_e_anchor = LogSummary(anchor_DIR,0)
    gop_e_anchor._analyze()
    gop_e_anchor.outputOverallResult(anchor_gop="1")
    
    
    #DIR1 = r".\_RC0_BGD1_LTR1_SCENE1_L_08_GOP_ltrnum_4\data"
    #print(DIR1)
    #gop_e = LogSummary(DIR1,0)
    #gop_e.outputResult_compare(gop_e_anchor.result,anchor_gop="8")
    
if __name__ =="__main__":main()
                    
                    
                    
