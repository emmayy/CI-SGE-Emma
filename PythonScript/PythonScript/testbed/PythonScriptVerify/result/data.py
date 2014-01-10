import glob
import re
import os

def parsefilename(filename):
    basename = os.path.basename(filename)
    rex = "([LM])_08_(S*_*)ltrnum_(\d)"

    result = re.search(rex,basename)
    if result:
        return result.groups()
    else:
        raise
    
def getstatisticsinfo(filename):
    if os.path.exists(filename):
        rex = "over all state : avg ratio:\s+(-?\d+.\d+) avg t0 ratio\s+(-?\d+.\d+)\s+avg fps ratio\s+(-?\d+.\d+)"
        with open(filename)as f:
            for line in f:
                result = re.search(rex,line)
                if result:
                    ratio,t0ratio,fpsratio = result.groups()
                    #print(ratio + "\t" + t0ratio + "\t" + fpsratio)
                    return (float(ratio),float(t0ratio),float(fpsratio))
            else: raise
        
def createdatadict(filepath):
    files = glob.glob(os.path.join(filepath,"*.txt"))
    datadict = dict()
    
    for file in files:
        #print(file)
        largechange,str_reuse,ltrnum = parsefilename(file)
        str_reuse = str_reuse == "S_"
        #print("largechange = %s,str_reuse = %s ,ltrnum = %d" %(largechange,str_reuse,int(ltrnum)))
        if largechange not in datadict.keys():
            datadict[largechange] = dict()
        if str_reuse not in datadict[largechange].keys():
            datadict[largechange][str_reuse] = dict()
        if ltrnum not in datadict[largechange][str_reuse].keys():
            datadict[largechange][str_reuse][ltrnum] = dict()
        ratio,t0ratio,fpsratio = getstatisticsinfo(file)
        datadict[largechange][str_reuse][ltrnum]["ratio"] = ratio
        datadict[largechange][str_reuse][ltrnum]["t0ratio"] = t0ratio
        datadict[largechange][str_reuse][ltrnum]["fpsratio"] = fpsratio
    else:
        return datadict

def summarizedata(datadict):
    print("scenechange \t str_reuse\tltrnum \t ratio \t fpsratio")
    for scenechangeratio in datadict.keys():
        for str_reuse in datadict[scenechangeratio].keys():
            for ltrnum in sorted(datadict[scenechangeratio][str_reuse].keys()):
                print("{scenechangeratio}".format(scenechangeratio = scenechangeratio), end = "\t")
                print("{str_reuse}".format(str_reuse = str_reuse ), end = "\t")
                print("{ltrnum}".format(ltrnum = ltrnum),end = "\t")
                print("{ratio}".format(ratio = datadict[scenechangeratio][str_reuse][ltrnum]["ratio"]),end = "\t")
                print("{fpsratio}".format(fpsratio = datadict[scenechangeratio][str_reuse][ltrnum]["fpsratio"]),end = "\t")
                print()
            else:print()
        else: print()
    else: print()
                      
def main():
    datadict = createdatadict(os.getcwd())
    summarizedata(datadict)
    

if __name__ == "__main__":main()
