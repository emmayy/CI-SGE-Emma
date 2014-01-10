import re
import math

class AnalyzeResult:
    def __init__(self,tid = 0):
        self.tid = tid
        self.ratio = 0
        self.frameCount = 0
        self.avgFrameSize = 0
        self.totalFrameSize = 0
        self.frameSizeList = []
    
    def analyze(self):
        self.frameCount = len(self.frameSizeList)
        self.totalFrameSize = sum(self.frameSizeList)        
        self.avgFrameSize = self.totalFrameSize / self.frameCount if self.frameCount else 0
        
class datafile:
    def __init__(self,filename):
        self.filename = filename
        self.frameSizeSum  = 0
        self.result = []
        self.Iframesize = []
        self.frameidcmapping = (0,8,16,17,18,19)
        self.frameidx = [[],[],[],[],[],[]]
        self.framenuminterval = [[],[],[],[],[],[]]
        self.framenuminterval_variance = []
        self.framenuminterval_number = []
        self.totalframecount = 0
        self.fps = 0
        self.psnr = 0
        self.bitrate = 0
        self.totalframes = 0
        self._analyze_total_status()
        self._analyze()

    def _calc_frame_interval(self,idc_idx):
        idx_list_length = len(self.frameidx[idc_idx])
        self.framenuminterval_number.append(idx_list_length)
        if idx_list_length > 2:
            frameidxintervallist = []
            for frm in range(idx_list_length-1):
                frameidxintervallist.append(self.frameidx[idc_idx][frm+1] - self.frameidx[idc_idx][frm])
            self.framenuminterval_variance.append(self._calc_variance(frameidxintervallist))
        else:
            self.framenuminterval_variance.append(-1)

            
    def _calc_variance(self,frameidxintervallist):
        list_length = len(frameidxintervallist)
        intervalave=math.fsum(frameidxintervallist)/list_length

        intervalvar = 0
        for idx in range(list_length):
            intervalvar += (frameidxintervallist[idx]**2 - intervalave**2)
        intervalvar = intervalvar/intervalave
        return intervalvar
        
    def _analyze_total_status(self):
        with open(self.filename) as f:
            rex_textline = ("PSNR:\s+(\d+).(\d+)\s","FPS:\s+(\d+).(\d+)\s","bitrate \(kbps\):\s+(\d+).(\d+)\s","Number of pictures encoded:\s+(\d+)\s")
            for line in f:
                for idx in range(4):
                    matchResult = re.search(rex_textline[idx],line)
                    if matchResult:
                        if idx==0:
                            self.psnr = float(matchResult.groups()[0] + "." + matchResult.groups()[1][0:4])
                        if idx==1:
                            self.fps = float(matchResult.groups()[0] + "." + matchResult.groups()[1][0:4])
                        if idx==2:
                            self.bitrate = float(matchResult.groups()[0] + "." + matchResult.groups()[1][0:2])
                        if idx==3:
                            self.totalframes = int(matchResult.groups()[0])
                            
    def _analyze(self):
        with open(self.filename) as f:
            rex = "frame_num\s+=\s+(\d+)\s+frame_idc\s+=\s+(\d+)\s+cur_tid\s+=\s+(\d+)\s+frame_size\s+=\s+(\d+)\s+"
            frameidx=0
            for line in f:
                matchResult = re.search(rex,line)
                if matchResult:
                    strFrameNum,strFrameIdc,strTid,strFrameSize = matchResult.groups()
                    frame_num = int(strFrameNum)
                    frame_idc = int(strFrameIdc)
                    cur_tid = int(strTid)
                    frame_size = int(strFrameSize)
                    while(cur_tid + 1 > len(self.result)):
                        result_tid = AnalyzeResult(cur_tid)
                        self.result.append(result_tid)
                    self.result[cur_tid].frameSizeList.append(frame_size)
                    if frame_idc == 0:
                        self.Iframesize.append(frame_size)
                    for idc_idx in range(len(self.frameidcmapping)):
                        if frame_idc <= self.frameidcmapping[idc_idx]:#LAYER
                            self.frameidx[idc_idx].append(frameidx)
                    frameidx = frameidx+1

        self.totalframecount = frameidx
        
        self.frameSizeSum  = 0
        for reusltTid in self.result:
            reusltTid.analyze()
            self.frameSizeSum  += reusltTid.totalFrameSize
        
        
        for reusltTid in self.result:
            reusltTid.ratio = reusltTid.totalFrameSize / self.frameSizeSum

        #for idc_idx in range(len(self.result)+2):
        #    self._calc_frame_interval(idc_idx)
      
    def getResult(self,tid):
        return self.result[tid] if tid <len(self.result) else None
    
