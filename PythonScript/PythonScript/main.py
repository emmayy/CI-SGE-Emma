import CEncoderConfig
import TestCase
import dataSummary
import os

def runAnchor():    
    config = CEncoderConfig.CEncoderConfig(LTR = True ,SEQ_DIR = seq_dir)
    config.exepath = ".\\bin\\L_08_GOP" #anchor exe path, change it if necessary
    suffix = os.path.basename(config.exepath)
    config.rootsuffix = "%s_anchor" %(suffix)
    config.addsuffix()
    config.printConfig()
    testcase = TestCase.TestCase(config,MaxProcess)
    testcase.run()
    return config.datapath,

def outputresult(config , outputTratio = True,comparewithanchor = True , compare_anchor = None, Anchor_gop = "1"):
    resultpath = ".\\result"
    if os.path.exists(resultpath) == False:
        os.mkdir(resultpath)
        
    print(config.datapath)
    gop_e = dataSummary.GOP_effiency(config.datapath, config.rc)
    outputfile = os.path.join(resultpath , config.rootsuffix + ".txt")
    with open(outputfile,"w") as f:
        if outputTratio: gop_e.outputTratio(outputTar = f)
        gop_e.outputOverallResult(outputTar = f, anchor_gop = Anchor_gop)
        if comparewithanchor and compare_anchor:
            gop_e._compareOverallResult(compare_anchor.result,outputTar = f,anchor_gop=Anchor_gop)
    if comparewithanchor and compare_anchor:
        gop_e._compareOverallResult(compare_anchor.result,anchor_gop= Anchor_gop)

MaxProcess = 1
RC_test = 0
Anchor_Gop = "1"
#seq_dir = r"D:\TestSequences\tmp3"
seq_dir = r"D:\TestSequences\HeadShoulders"
PSNRcalc_dir = r"C:\Users\gpu4\Desktop\openh264\PythonScript_win\PythonScript\testbed"#folder that stores "H264AVCDecoderLibTestStatic.exe" and "PSNRStatic.exe"

def main(runAnchor = False,comparewithanchor = True,output = True, PSNRcalc = True):     

    if comparewithanchor and runAnchor: runAnchor()
    
    config_list = [] 
    exepath_list = [".\\bin\\openH264_v460"]#,".\\bin\\OpenH264_v60"]
    
    if comparewithanchor:
        anchor_DIR = r"D:\workspace\GOP_test\_RC0_BGD1_LTR0_SCENE1_\data"
        print(anchor_DIR)
        compare_anchor = dataSummary.GOP_effiency(anchor_DIR,RC_test)
        
    for exepath in exepath_list:
        suffix = os.path.basename(exepath)
        config = CEncoderConfig.CEncoderConfig(LTR = True ,SEQ_DIR = seq_dir)
        config.exepath = exepath
        config.PSNRcalc = PSNRcalc
        config.PSNRcalc_dir = PSNRcalc_dir
        config.ltrnum = (1,)   
        config.gopList = (1,)   #config.gopList = (1,2,4,8)
        config.qpList = (20,22,27,32,37,42)           
        config.rc = RC_test  #config.rc = (0,1) rc1的时候 config.brList = (256,384,512,768,1024,1536,2048)
        config.brList = (256,384,512,768,1024,1536,2048)  #rc1的时候 config.brList = (256,384,512,768,1024,1536,2048)
        config.me16x16 = 3
        config.me8x8 = 7
        config.fmehash = 0
        config.rootsuffix = "%s_SingleThread_RC%d" %(suffix,RC_test)
        config.addsuffix()
        config_list.append(config)
    
    for config in config_list:
        config.printConfig()
        testcase = TestCase.TestCase(config,MaxProcess)
        testcase.run()
        
        if output:
            if comparewithanchor:
                outputresult(config,outputTratio = True,comparewithanchor = True,compare_anchor = compare_anchor,Anchor_gop = Anchor_Gop)
            else:
                outputresult(config,outputTratio = True,comparewithanchor = False,Anchor_gop = Anchor_Gop)
        
if __name__ == "__main__":
    main(runAnchor = False, comparewithanchor = False , output = True, PSNRcalc = True)
