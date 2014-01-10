import os
import MyProcessPool

def createDataDir(dataPath):
    print(dataPath)
    if os.path.exists(dataPath) == False:
        os.mkdir(dataPath) 

import re

def getResolution(filename):
    rex = "(\d+)[xX](\d+)"
    result = re.search(rex,filename)
    if result:
        width,height = result.groups()
        return (int(width),int(height))
    else:
        return (0,0)

  
class OneCase:
    def __init__(self,index,file, qp,target_br,gop,datapath,filepath,config,exename  = "encConsole.exe" ):
        self.exepath = config.exepath
        self.datapath = datapath
        self.filepath = filepath
        self.filebasename = os.path.basename(file)
        self.index = index
        
        width,height = getResolution(self.filebasename)
        exefile = exename 
        cfgfile = "wbxenc.cfg" 
        tracefile = os.path.join(self.datapath,self.filebasename[:-4] + ".txt")
        bsfile = os.path.join(self.filepath,self.filebasename[:-4] + ".264")
        self.cmdline = str(exefile)+ "  " + cfgfile + " -bf %-50s " %bsfile + " -lqp 0 %d" %qp + " -ltarb 0 %d" %target_br + " -scene %d" %config.scenechange+ " -rc %d" %config.rc + " -bgd %d"%config.bgd  + " -ltr %d" %config.ltr + " -numltr  %d" %config.ltrnum + " -gop %4d "%gop +" -me16x16 %4d -me8x8 %4d -fmehash  %4d "%(config.me16x16,config.me8x8,config.fmehash) + " -sw 0 %4d -sh 0 %4d " %(width,height) + " -org 0 %-90s " %file + ">" + tracefile
        #self.cmdline = "lencod.exe"+ "  " + " -f encoder.cfg" + " -p InputFile=\"%-50s\" " %file + "-p OutputFile=\"%-50s\" " %bsfile + " -p SourceWidth=%4d -p SourceHeight=%4d " %(width,height) + ">" + tracefile

        self.PSNRcalc = config.PSNRcalc
        if (self.PSNRcalc):
            self.decfile = os.path.join(self.filepath,self.filebasename[:-4] + "dec.yuv")
            decexepath = config.PSNRcalc_dir
            decexename = "ldecod.exe"
            decexefile = os.path.join(decexepath,decexename)
            self.deccmdline = str(decexefile)+ " -i %-50s " %bsfile + " -o %-90s " %self.decfile
            
            calcexename = "PSNRStatic.exe"
            calcexefile = os.path.join(decexepath,calcexename) 
            self.calccmdline = str(calcexefile)+ " %4d %4d " %(width,height) + " %-90s " %file + " %-90s " %self.decfile + "0 0 %s 30 " %bsfile + ">>" + tracefile
            #self.calccmdline = str(calcexefile)+ " %4d %4d " %(width,height) + " rec_layer2.yuv " + " %-90s " %self.decfile + ">>" + tracefile
        else:
            self.deccmdline = ""
            self.decfile = ""
            self.calccmdline = ""           
                
            
    def getParamList(self):
        return [self.cmdline,self.filebasename,self.index,self.deccmdline,self.decfile,self.calccmdline,self.PSNRcalc]

def logstr(logstr,logQueue = None):
    if logQueue:     logQueue.put(logstr)
    else:    print(logstr)

def taskfunc(onecase,logQueue = None):
    #[self.cmdline, self.filebasename, self.index]
    try:
        os.system(onecase[0])
    except:   
        logstr("encode failed! : "+ onecase[1],logQueue = logQueue)
        return
    logstr("index : %-5d" %(onecase[2]) + onecase[1] , logQueue = logQueue)    

    if (onecase[6]):
        try:
            os.system(onecase[3])
        except:
            logstr("decode failed! : "+ onecase[1],logQueue = logQueue)
            return
        logstr(" decode index : %-5d" %(onecase[2]) + onecase[1] , logQueue = logQueue)

        try:
            os.system(onecase[5])
        except:
            logstr("PSNR calc failed! : "+ onecase[1],logQueue = logQueue)
            return
        logstr(" PSNR index : %-5d" %(onecase[2]) + onecase[1] , logQueue = logQueue)
        
        if os.path.exists(onecase[4]):
            os.remove(onecase[4])
        
class TestCase:
    def __init__(self,config,MaxProcess = 1):
        self._config = config
        self._caseList = []
        self.MaxProcess = MaxProcess
        self._createdirs(self._config)  
        self._generateCaseList(self._config)          

    def _createdirs(self,config):   
        createDataDir(config.rootpath)
        createDataDir(config.datapath)
        createDataDir(config.filepath)
        for GOP in config.gopList:
            DATA_GOP_DIR = config.datapath + os.path.sep + "GOP_" +str(GOP)
            createDataDir(DATA_GOP_DIR)
            FILE_GOP_DIR = config.filepath + os.path.sep + "GOP_" +str(GOP)
            createDataDir(FILE_GOP_DIR)
            
            if (config.rc==0):
                for QP in config.qpList:
                    DATA_QP_DIR = DATA_GOP_DIR + os.path.sep + "QP_" + str(QP) 
                    createDataDir(DATA_QP_DIR)
                    FILE_QP_DIR = FILE_GOP_DIR + os.path.sep + "QP_" + str(QP) 
                    createDataDir(FILE_QP_DIR)
            else:#rc==1
                for BR in config.brList:
                    DATA_BR_DIR = DATA_GOP_DIR + os.path.sep + "BR_" + str(BR) 
                    createDataDir(DATA_BR_DIR)
                    FILE_BR_DIR = FILE_GOP_DIR + os.path.sep + "BR_" + str(BR) 
                    createDataDir(FILE_BR_DIR)         
            
        
                    
    def _generateCaseList(self,config):
        idx = 0
        for GOP in config.gopList:
            DATA_GOP_DIR = config.datapath + os.path.sep + "GOP_" +str(GOP)
            FILE_GOP_DIR = config.filepath + os.path.sep + "GOP_" +str(GOP)

            if (config.rc==0):
                for QP in config.qpList:
                    DATA_QP_DIR = DATA_GOP_DIR + os.path.sep + "QP_" + str(QP) 
                    FILE_QP_DIR = FILE_GOP_DIR + os.path.sep + "QP_" + str(QP) 
                    
                    for ifile in config.files:
                        onecase = OneCase(index = idx ,file = ifile,qp = QP,target_br=1000000,gop = GOP,datapath = DATA_QP_DIR,filepath = FILE_QP_DIR,config = config,exename = config.exename)
                        self._caseList.append(onecase)
                        idx += 1
            else:
                for BR in config.brList:
                    DATA_BR_DIR = DATA_GOP_DIR + os.path.sep + "BR_" + str(BR) 
                    FILE_BR_DIR = FILE_GOP_DIR + os.path.sep + "BR_" + str(BR) 
                    
                    for ifile in config.files:
                        onecase = OneCase(index = idx ,file = ifile,qp = 28,target_br=BR,gop = GOP,datapath = DATA_BR_DIR,filepath = FILE_BR_DIR,config = config,exename = config.exename)
                        self._caseList.append(onecase)
                        idx += 1   
                   
    def run(self):
        curpath = os.getcwd()
        os.chdir(self._config.exepath)
        processpool = MyProcessPool.MyProcessPool(self._caseList,taskfunc,MaxProcess = self.MaxProcess)
        processpool.run()
        os.chdir(curpath)
