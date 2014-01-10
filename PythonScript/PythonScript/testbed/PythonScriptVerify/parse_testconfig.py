
import re

def getseqdir(openedfile):
    rex = r"SEQ_DIR\s+([:\w\/\\\.]+)"
    for line in openedfile: 
        result = re.search(rex,line)
        if result:
            return result.groups()[0]
    else:
        return  None
        
def getmaxprocessnum(openedfile):
    rex = r"MAX_PROCESS_NUMBER\s+(\d+)"
    for line in openedfile: 
        result = re.search(rex,line)
        if result:
            return result.groups()[0]
    else:
        return  None

def getlist(openedfile,rexkey,rexvalue):
    for line in openedfile: 
        result = re.search(rexkey,line)
        if result:
            resultvalue = re.findall(rexvalue,line)
            return resultvalue
    else:
        return  None    
    
def getexepathlist(openedfile):
    rexkey = r"EXEPATH\sLIST"
    rexvalue = r"[:\w\/\.\\\_]+"
    return getlist(openedfile,rexkey,rexvalue)

def getgoplist(openedfile):
    rexkey = r"GOP\s+LIST"
    rexvalue = r"\d+"
    return getlist(openedfile,rexkey,rexvalue)  
    
def getqplist(openedfile):
    rexkey = r"QP\s+LIST"
    rexvalue = r"\d+"
    return getlist(openedfile,rexkey,rexvalue)

def getltrenablelist(openedfile):
    rexkey = r"LTR\s+SWITCH\s+LIST"
    rexvalue = r"\d+"
    return getlist(openedfile,rexkey,rexvalue)
 
def getltrnumlist(openedfile):
    rexkey = r"LTR\s+NUMBER\s+LIST"
    rexvalue = r"\d+"
    return getlist(openedfile,rexkey,rexvalue)  

def parsetestconfig(filename):
    with open(filename) as f:
        seqdir = getseqdir(f)
        print(seqdir)
        
        max_process = getmaxprocessnum(f)
        print(max_process)
        
        exepathlist = getexepathlist(f)
        print(exepathlist)
        
        goplist = getgoplist(f)
        print(goplist)
        
        ltrenablelist = getltrenablelist(f)
        print(ltrenablelist)
        ltrnumlist = getltrnumlist(f)
        print(ltrnumlist)
        



testconfigfile = ".\\test_config.txt"
if __name__ == "__main__":  
    parsetestconfig(testconfigfile)
    