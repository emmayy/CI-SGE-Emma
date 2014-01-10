import os
import glob
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

        self.idcfile = os.path.join(bsfile + ".idc")

        self.bSwitchSimulator = config.bSwitchSimulator
        if (self.bSwitchSimulator):
            
            switchexefile = os.path.join(config.Switch_dir,"SwitchSimulator.exe")
            bsoutfile = os.path.join(bsfile + "_out.264")
            self.callswitchcmdline = str(switchexefile)+ " %s " %bsfile + " %s " %self.idcfile + " %s " %bsoutfile   
            self.switchlogfile = os.path.join(bsfile + "_out.log")

            #bsoutyuvfile = os.path.join(bsoutfile + ".yuv")
            decexefile = os.path.join(config.JMdec_dir,"ldecod.exe")
            self.callJMDECcmdline = str(decexefile)+ " -i %s " %bsoutfile + "-o jmdec.yuv"
        else:
            self.callswitchcmdline = ""
            self.switchlogfile = ""
            self.callJMDECcmdline = ""
                
            
    def getParamList(self):
        return [self.cmdline,self.filebasename,self.index,self.callswitchcmdline,self.idcfile,self.switchlogfile,self.callJMDECcmdline,self.bSwitchSimulator]

def logstr(logstr,logQueue = None):
    if logQueue:     logQueue.put(logstr)
    else:    print(logstr)

def taskfunc(onecase,logQueue = None):
    #[self.cmdline, self.filebasename, self.index]
    logstr("index : %-5d" %(onecase[2]) + onecase[1] , logQueue = logQueue)    

    try:
        logstr("begin encoding & renaming frame_idc! : "+ onecase[1],logQueue = logQueue)
        if os.path.exists("frame_idc.idc"):
            os.remove("frame_idc.idc")
        os.system(onecase[0])
        if os.path.exists(onecase[4]):
            os.remove(onecase[4])
        os.rename("frame_idc.idc",onecase[4])
        logstr("finish encoding & renaming frame_idc! : "+ onecase[1],logQueue = logQueue)
    except:   
        logstr("encode failed! : "+ onecase[1],logQueue = logQueue)
        return

    if (onecase[7]):
        try:
            logstr("begin switch simulator! : "+ onecase[1],logQueue = logQueue)

            if os.path.exists("switch_output.log"):
                os.remove("switch_output.log")
            if os.path.exists(onecase[5]):
                os.remove(onecase[5])
                
            os.system(onecase[3])
            os.rename("switch_output.log",onecase[5])
            logstr("finish switch simulator "+ onecase[1],logQueue = logQueue)
        except:
            logstr("switch simulator failed! : "+ onecase[3],logQueue = logQueue)
            return
        #logstr(" switch simulator : %-5d" %(onecase[2]) + onecase[1], logQueue = logQueue)

    if (onecase[7]):
        try:
            logstr("begin JM decoder! : "+ onecase[1],logQueue = logQueue)
            os.system(onecase[6])

            os.system("del *.yuv")
            logstr("finish JM decoder! : "+ onecase[1],logQueue = logQueue)
        except:
            logstr("JM decoder failed! : "+ onecase[6],logQueue = logQueue)
            os.system("del *.exe")#if JM decoder failed, delete exe to stop further testing?
            return
        #logstr(" JM decoder: %-5d" %(onecase[2]) + onecase[1], logQueue = logQueue)
         
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
