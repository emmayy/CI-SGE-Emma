import os


                                           
class CEncoderConfig:
    def __init__(self,RC = 0, BGD = 1,LTR = 0,SCENECHANGE = 1,GOPLIST = (1,2,4),QPLIST = range(20,40,4),SEQ_DIR = "D:\\Sequence\\SCC_SEQ_NODUPLICATE",exename = "encConsole.exe",rootsuffix = ""):
        self.rc = RC
        self.bgd = BGD
        self.ltr = LTR
        self.scenechange = SCENECHANGE
        self.exename = exename
        self.exepath = ".\\bin"
        self.gopList = list(GOPLIST)
        self.qpList = QPLIST
        self.rootsuffix = rootsuffix
        self.rootpath = os.path.join(os.getcwd(),"_RC%d_BGD%d_LTR%d_SCENE%d_" %(self.rc,self.bgd,self.ltr,self.scenechange) )
        self.datapath = os.path.join(self.rootpath,"data")
        self.filepath = os.path.join(self.rootpath,"files")
        self.seq_dir = SEQ_DIR
        self.files = []
        self._generateFileList()

    def _generateFileList(self):
        for roots,dirs,files in os.walk(self.seq_dir):
            if dirs == []:
                for srcfilename in files:
                    if srcfilename[-4:] == ".yuv":
                        fullfilename = os.path.join(roots,srcfilename)
                        self.files.append(fullfilename)
                    
    def addsuffix(self):
        self.rootpath += self.rootsuffix
        self.datapath = os.path.join(self.rootpath,"data")
        self.filepath = os.path.join(self.rootpath,"files")
        
    def printConfig(self):
        lineEnd = "\n"
        print()
        print("sequence dir : %s " %self.seq_dir,end = lineEnd) 
        print("RC %d" %self.rc, end = lineEnd )
        print("BGD %d" %self.bgd,end = lineEnd )
        print("LTR %d" %self.ltr,end = lineEnd )
        print("SCENECHANGE %d" %self.scenechange,end = lineEnd )
        print("gopList : " + str(self.gopList),end = lineEnd )
        print("qpList :"  + str(self.qpList),end = lineEnd )
        print("rootpath : %s " %self.rootpath,end = lineEnd )
        print("\n")
                  
