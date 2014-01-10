
import os
import re
import dataAnalyze
import glob
import math

class data_collect:
    def __init__(self,DIR):
        self.DIR = DIR
        self.result = dict()        
        self._analyze()
        
    def _getQopAndQP(self,roots):
        rex = "GOP_(\d)\\\\QP_(\d+)"        
        matchResult = re.search(rex,roots)
        gop = '0'
        qp = '0'
        if matchResult:
            gop,qp = matchResult.groups()
        return (gop,qp)
    
    def _analyze(self):
        #print(self.DIR)
        for roots,dirs,files in os.walk(self.DIR):
            if dirs == []:
                gop,qp = self._getQopAndQP(roots)
                if gop not in self.result.keys():   self.result[gop] = dict()                   
                if qp not in self.result[gop].keys():   self.result[gop][qp] = dict()
                for dfile in files:                    
                    if dfile != "wels_svc_encoder_trace.txt":
                        onefile = dataAnalyze.datafile(os.path.join(roots,dfile))                       
                        self.result[gop][qp][dfile] = onefile

def outputonedfile(dfile,file = os.sys.stdout):
        frameSizeSum = dfile.frameSizeSum
        tidstr = "  "
        tframecountstr = "  "
        tsizestr = "  "
        tratiostr = "  "
        #print("GOP = %s" %gop,end = " ",file = outputTar )

        outputstr = "%-10d\t" %frameSizeSum
                    
        for tid in range(len(dfile.result)):
            framecount = dfile.getResult(tid).frameCount
            tsize = dfile.getResult(tid).totalFrameSize
            tratio = tsize/frameSizeSum
            tidstr += "t%d " %tid
            tframecountstr += "%-4d\t" %framecount 
            tsizestr += "%-10d\t" %tsize
            tratiostr += "%1.3f\t" %tratio                        
        else:
            outputstr += tframecountstr +  tsizestr + "\t"#+ tratiostr
            #print(tidstr + tframecountstr +  tsizestr + tratiostr ,file = outputTar)

        return outputstr

def gettardir():
    dirlist = glob.glob("*")
    rex = "_RC\d_BGD\d_LTR\d_SCENE\d"
    tardirlist =[]
    for direction in dirlist:
        result = re.search(rex,direction)
        if result: tardirlist.append(direction)
    return tardirlist

def main():
    tardirlist = gettardir() 
    print(tardirlist)
    combined_result = dict()
    tablehead = dict()
    
    for tardir in tardirlist:
        dirs = os.path.join(tardir,"data")
        print(dirs)
        collecter = data_collect(dirs)

        for gop in sorted(collecter.result.keys()):
            if gop not in combined_result.keys(): combined_result[gop] = dict()
            if gop not in tablehead.keys(): tablehead[gop] = "sequence\t"
            for qp in sorted(collecter.result[gop].keys()):
                tlayercount = int(math.log(int(gop))/math.log(2)) + 1
                tablehead[gop] += tardir[22:] + "_gop_" + gop + "\t"*(tlayercount*2 + 2)
                for dfile in collecter.result[gop][qp].keys():
                    if dfile not in combined_result[gop].keys():     combined_result[gop][dfile] = ""
                    combined_result[gop][dfile] += outputonedfile(collecter.result[gop][qp][dfile])
        
        #break
    for gop in combined_result.keys():
        print(tablehead[gop] )
        for dfile in combined_result[gop].keys():
            print("%-36s " %dfile[:-16],end="\t" )
            print(combined_result[gop][dfile])
            
    for gop in combined_result.keys():  
        with open("flexibleGOP_result"+gop+".txt","w") as f:
            print(tablehead[gop] , file = f)
            for dfile in combined_result[gop].keys():
                print("%-36s " %dfile[:-16],end="\t" ,file =f)
                print(combined_result[gop][dfile], file = f)
if __name__ == "__main__":  main()

