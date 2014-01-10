import CEncoderConfig
import TestCase
import dataSummary
import logSummary
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

    #bs info    
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

    #switch log
    log_sum = logSummary.LogSummary(config.filepath, config.rc)
    outputfile = os.path.join(resultpath , config.rootsuffix + "_switch.log")
    with open(outputfile,"w") as f:
        log_sum.outputOverallResult(outputTar = f, anchor_gop = Anchor_gop)

MaxProcess = 1
RC_test = 0
Anchor_Gop = "8"
#seq_dir = r"F:\TestSequences\PebbleBeach_scc_testset"
seq_dir = r"F:\TestSequences\tmp"
#seq_dir = r"C:\@Applications\TestSequences\svc\640x360"
Switch_dir = r"F:\TeamCodes\PebbleBeach_WelsRuby_Upgrade\welsruby\testbed\SwitchSimulator\Build\Win32\SwitchSimulator\Debug"
#folder that stores "Switch Simulator"
JMdec_dir = r"C:\@Codes\Tools\@EXEs"
#folder that stores "JM decoder"

def main(runAnchor = False,comparewithanchor = True,output = True, bSwitchSimulator = True):     

    if comparewithanchor and runAnchor: runAnchor()
    
    config_list = [] 
    exepath_list = [".\\bin\\test"]
    
    if comparewithanchor:
        anchor_DIR = r"D:\workspace\GOP_test\_RC0_BGD1_LTR0_SCENE1_\data"
        print(anchor_DIR)
        compare_anchor = dataSummary.GOP_effiency(anchor_DIR,RC_test)
        
    for exepath in exepath_list:
        suffix = os.path.basename(exepath)
        for ltrnum in range(4,5,1):
            me16x16 = 3
            me8x8 = 7
            fmehash = 0
            #for me16x16 in (3,):
            #    for me8x8 in (7,):#(1,2,4,3,7,5,8,16):
            #        if me16x16==1 and me8x8==1:#not need to run anchor again
            #            continue
            #        if (me16x16>=4 and me16x16<8) and (me8x8>=4 and me8x8<8):#no FME in both ME16 and ME8
            #            continue
            #        if ((me16x16>=4 and me16x16<8) and me8x8>=8) or (me16x16>=8 and (me8x8>=4 and me8x8<8)):#FME does not cooperate with more complex ME
            #            continue
            #        if me16x16==16 and (me8x8>1 and me8x8<16): #ME16==FULL ME does not cooperate with more complex ME
            #            continue
            #        for fmehash in (0,):#(0,1,2,3):
            #            if fmehash!=0 and (me16x16<4 or me16x16>7) and (me8x8<4 or me8x8>7):#fmehash is only used to cooperate with FME
            #                continue
            config = CEncoderConfig.CEncoderConfig(LTR = True ,SEQ_DIR = seq_dir)
            config.exepath = exepath
            config.bSwitchSimulator = bSwitchSimulator
            config.Switch_dir = Switch_dir
            config.JMdec_dir = JMdec_dir
            config.ltrnum = ltrnum    
            config.gopList = (8,)
            config.qpList = (26,)           
            config.rc = RC_test
            config.brList = (400,1000,1500,2500)
            config.me16x16 = me16x16
            config.me8x8 = me8x8
            config.fmehash = fmehash
            config.rootsuffix = "%s_me16_%d_me8_%d_fmehash_%d_ltrnum_%d" %(suffix,me16x16,me8x8,fmehash,ltrnum)
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
    main(runAnchor = False, comparewithanchor = False , output = True, bSwitchSimulator = True)
