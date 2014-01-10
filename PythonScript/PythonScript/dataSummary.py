
import os
import re
import dataAnalyze

class GOP_effiency:
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
                    if dfile != "wels_svc_encoder_trace.txt":
                        onefile = dataAnalyze.datafile(os.path.join(roots,dfile))
                        if dfile not in self.result.keys() :self.result[dfile] = dict()
                        if gop not in self.result[dfile].keys() : self.result[dfile][gop] = dict()
                        if qp not in self.result[dfile][gop].keys() : self.result[dfile][gop][qp] = onefile


    def calcuTratio(self,outputTar = os.sys.stdout,anchor_gop="1"):
        print("{seq:35}\t{qp:3}\t{gop:3}\t{TotalSize:10}\t{t0size:10}\t{fps:35}\t{bitrate:35}\t{psnr:35}\t".format(seq = "sequence",qp = "qp",gop = "gop",TotalSize = "TotalSize",t0size = "t0size",fps = "FPS",bitrate = "BitRate",psnr = "YPSNR"),file = outputTar)
         
        ratiolist = [[],[],[],[],[],[],[],[],[],[]]
        t0ratiolist = [[],[],[],[],[],[],[],[],[],[]]
        for dfile in self.result.keys():
            filename = dfile
            for gop in sorted(self.result[dfile].keys()):
                for qp in self.result[dfile][gop].keys():
                    print(" %-35s\t%3s\t" %(filename[:-16],str(qp)),end = "",file = outputTar)
                    filename = ""
                    gop1frameSizeSum = self.result[dfile][anchor_gop][qp].frameSizeSum             
                    gop1size = self.result[dfile][anchor_gop][qp].getResult(0).totalFrameSize if self.result[dfile][anchor_gop][qp].getResult(0).totalFrameSize else 0    

                    frameSizeSum = self.result[dfile][gop][qp].frameSizeSum 
                    gopsize = self.result[dfile][gop][qp].getResult(0).totalFrameSize

                    if gop1frameSizeSum!=0:
                        ratio = frameSizeSum/gop1frameSizeSum
                        ratiolist[int(gop)].append(ratio)

                    if gop1size!=0:
                        t0Ratio = gopsize/gop1size
                        t0ratiolist[int(gop)].append(t0Ratio)

                    if self.result[dfile][gop][qp].totalframecount!=0:
                        framecount = self.result[dfile][gop][qp].totalframecount
                    else: framecount = self.result[dfile][gop][qp].totalframes
                            
  
    def _outputonefileTratio(self,datafile,outputTar = os.sys.stdout):
        frameSizeSum = datafile.frameSizeSum
        tidstr = "  "
        tframecountstr = "  "
        tsizestr = "  "
        tratiostr = "  "
        tBrStr = "  "
        ttltime = (datafile.totalframes/10.0)
                    
        for tid in range(len(datafile.result)):
            framecount = datafile.getResult(tid).frameCount
            tsize = datafile.getResult(tid).totalFrameSize
            tratio = tsize/frameSizeSum
            tidstr += "t%d\t" %tid
            tframecountstr += "%-4d\t" %framecount 
            tsizestr += "%-10d\t" %tsize
            tratiostr += "%1.3f\t" %tratio
            if ttltime!=0: tBrStr += "%1.3f\t" %((tsize/ttltime)/1000)
            else: tBrStr += "totalframes=%d\t" %(datafile.totalframes)

        else:
            return ("T_id\t"+ tidstr + "T_Framecount\t"+ tframecountstr +  "T_Size\t" + tsizestr + "T_Ratio\t" + tratiostr + "T_BR\t" + tBrStr )

    def _outputonefileTdistrib(self,datafile,outputTar = os.sys.stdout):
        varstr = "num/var\t"
        tlayers = len(datafile.result)
        for idx in range(tlayers+2):
            varstr += "%d\t" %datafile.framenuminterval_number[idx]
            varstr += "%d\t" %datafile.framenuminterval_variance[idx]
        return varstr

    def outputTratio(self,outputTar = os.sys.stdout):   
        for dfile in self.result.keys():
            for gop in sorted(self.result[dfile].keys()):
                for qp in self.result[dfile][gop].keys():
                    printstr=" %-32s\tQP =\t%3s\t GOP =\t%s\t" %(dfile[:-16],str(qp),gop)
                    printstr += self._outputonefileTratio(self.result[dfile][gop][qp], outputTar = outputTar )
                    #printstr += self._outputonefileTdistrib(self.result[dfile][gop][qp], outputTar = outputTar )
                    print(printstr,file = outputTar )  
                                        
    def outputOverallResult(self,outputTar = os.sys.stdout,anchor_gop="1"):
        print("{seq:35}\t{qp:3}\t{gop:3}\t{fps:35}\t{bitrate:35}\t{psnr:35}\t".format(seq = "sequence",qp = "qp",gop = "gop",TotalSize = "TotalSize",t0size = "t0size",fps = "FPS",bitrate = "BitRate",psnr = "YPSNR"),file = outputTar)
         
        ratiolist = [[],[],[],[],[],[],[],[],[],[]]
        t0ratiolist = [[],[],[],[],[],[],[],[],[],[]]
        for dfile in self.result.keys():
            filename = dfile
            for gop in sorted(self.result[dfile].keys()):
                for qp in self.result[dfile][gop].keys():
                    print(" %-35s\t%3s\t" %(filename[:-16],str(qp)),end = "",file = outputTar)
                    filename = ""
                            
                    fps = self.result[dfile][gop][qp].fps
                    psnr = self.result[dfile][gop][qp].psnr
                    bitrate = self.result[dfile][gop][qp].bitrate
                    framecount = self.result[dfile][gop][qp].totalframes
                    print("%3d\t" %int(gop) + "%-5.3f\t" %fps + "%-5.3f\t" %bitrate + "%-10.3f\t" %psnr + "%6d\t" %(framecount),end = "\n",file = outputTar)
                    
    def _compare_dfile(self,srcfile,anchorfile,outputtar = os.sys.stdout):
        anchor_size = anchorfile.frameSizeSum
        anchor_framecount = anchorfile.framecount
        anchor_fps = anchorfile.fps
        
        src_size = srcfile.frameSizeSum
        src_framecount = srcfile.framecount
        src_fps = srcfile.fps
        
        size_ratio = src_size/anchor_size
        framecount_ratio = src_framecount/anchor_framecount
        fps_ratio = src_fps/anchor_fps
        print("%-35s %-1.3f %-1.3f " %(srcfile.filename,size_ratio,framecount_ratio,fps_ratio),file = outputtar)
        
    def _compareTratio(self,anchor = None,outputTar = os.sys.stdout,anchor_gop="1"):
        compareAnchor = anchor if anchor else self.result        
        print("{0:60} :\t{1}\t{2}\t".format("sequence","ratio","t0Ratio"),file = outputTar)
        avg_ratio_list = []
        avg_t0ratio_list = []

        for dfile in self.result.keys():
            ratioList = []
            t0RatioList = []
            for gop in self.result[dfile].keys():
                for qp in self.result[dfile][gop].keys():
                    anchor_framesize = compareAnchor[dfile][anchor_gop][qp].frameSizeSum
                    anchor_gop1size = compareAnchor[dfile][anchor_gop][qp].getResult(0).totalFrameSize
                    
                    frameSizeSum = self.result[dfile][gop][qp].frameSizeSum
                    ratio = frameSizeSum/anchor_framesize
                    ratioList.append(ratio)
                     
                    gopsize = self.result[dfile][gop][qp].getResult(0).totalFrameSize                    
                    t0Ratio = gopsize/anchor_gop1size
                    t0RatioList.append(t0Ratio)              
                      
            avg_t0Ratio = sum(t0RatioList)/len(t0RatioList) - 1
            avg_t0ratio_list.append(avg_t0Ratio)                    
            print("%-60s :\t%8.3f\t%8.3f\t" %(dfile,avg_ratio,avg_t0Ratio),file = outputTar)
       
        overall_avg_t0Ratio = sum(avg_t0ratio_list)/len(avg_t0ratio_list) 
        print("over all state : avg ratio:\t%8.3f\tavg t0 ratio\t%8.3f\t" %(overall_avg_ratio,overall_avg_t0Ratio),file = outputTar)

    def _compareOverallResult(self,anchor = None,outputTar = os.sys.stdout,anchor_gop="1"):
        compareAnchor = anchor if anchor else self.result        
        print("{0:60} :\t{1}\t{2}\t{3}\t".format("sequence","ratio","fpsRatio","psnrDelta"),file = outputTar)
        avg_ratio_list = []
        avg_fpsRatio_list = []
        avg_psnrDelta_list = []
        for dfile in self.result.keys():
            ratioList = []
            fpsRatioList = []                       
            psnrDeltaList = []  
            for gop in self.result[dfile].keys():
                for qp in self.result[dfile][gop].keys():
                    anchor_fps = compareAnchor[dfile][anchor_gop][qp].fps
                    anchor_psnr = compareAnchor[dfile][anchor_gop][qp].psnr
                     
                    fps = self.result[dfile][gop][qp].fps
                    fps_ratio = fps/anchor_fps
                    fpsRatioList.append(fps_ratio)

                    psnr = self.result[dfile][gop][qp].psnr
                    psnr_delta = psnr-anchor_psnr
                    psnrDeltaList.append(psnr_delta)
                    
            avg_ratio = sum(ratioList)/len(ratioList) - 1
            avg_ratio_list.append(avg_ratio)
            
            avg_fpsRatio = sum(fpsRatioList)/len(fpsRatioList) - 1
            avg_fpsRatio_list.append(avg_fpsRatio)

            avg_psnrDelta = sum(psnrDeltaList)/len(psnrDeltaList)
            avg_psnrDelta_list.append(avg_psnrDelta)
            
            print("%-60s :\t%8.3f\t%8.3f\t%8.3f\t" %(dfile,avg_ratio,avg_fpsRatio,avg_psnrDelta),file = outputTar)

        overall_avg_ratio = sum(avg_ratio_list)/len(avg_ratio_list)         
        overall_avg_fpsRatio = sum(avg_fpsRatio_list)/len(avg_fpsRatio_list) 
        overall_avg_psnrDelta = sum(avg_psnrDelta_list)/len(avg_psnrDelta_list) 
        
        print("over all state : avg ratio:\t%8.3f\tavg fps ratio \t%8.3f\tavg psnrDelta \t%8.3f\t" %(overall_avg_ratio,overall_avg_fpsRatio,overall_avg_psnrDelta),file = outputTar)


def main():
   
    anchor_DIR = r".\_RC0_BGD1_LTR1_SCENE1_Anchor_official_v3848_allI_me16_3_me8_7_fmehash_0_ltrnum_4\data"
    print(anchor_DIR)
    gop_e_anchor = GOP_effiency(anchor_DIR,0)
    gop_e_anchor.outputResult(anchor_gop="1")
    
    
    #DIR1 = r".\_RC0_BGD1_LTR1_SCENE1_L_08_GOP_ltrnum_4\data"
    #print(DIR1)
    #gop_e = GOP_effiency(DIR1,0)
    #gop_e.outputResult_compare(gop_e_anchor.result,anchor_gop="8")
    
if __name__ =="__main__":main()
                    
                    
                    
