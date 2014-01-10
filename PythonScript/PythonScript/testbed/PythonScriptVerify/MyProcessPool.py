import threading
import multiprocessing
import time

def processfunc(taskfunc,sharedQueue,globallock,logQueue):
    while(True):
        #with globallock:
        if sharedQueue.qsize() == 0 : break
        try:
            onecase = sharedQueue.get()
        except:
            break
        sharedQueue.task_done()               
        taskfunc(onecase,logQueue)
            
class MyProcessPool:
    def __init__(self,cmdList,taskfunc,MaxProcess = 1):
        self.task = taskfunc
        self.corecount = self.getCoreCount()        
        self.processlist = []
        self.globallock = multiprocessing.Lock()
        self.sharedQueue = multiprocessing.JoinableQueue()
        self.logQueue = multiprocessing.JoinableQueue()
        self.logthread = None
        
        for cmd in cmdList:     self.sharedQueue.put(cmd.getParamList());
            
        self.MaxProcess = min(self.corecount,MaxProcess,self.sharedQueue.qsize())
        
        if self.MaxProcess > 1:
            self._createProcesses(processfunc,taskfunc,self.sharedQueue,self.globallock,self.logQueue)
              
        print("sharedList len is %d  MaxProcess = %d "%(self.sharedQueue.qsize(),self.MaxProcess) )
    
        self.logthread = threading.Thread(target = self._recvsubprocesslog , name = "logthread")
        
    def _createProcesses(self,processfunc,taskfunc,sharedQueue,globallock,logQueue):        
        for x in range(self.MaxProcess):            
            sprocess = multiprocessing.Process(target = processfunc ,args = (taskfunc,sharedQueue,globallock,logQueue),name = "process"+str(x))
            self.processlist.append(sprocess)

    def _recvsubprocesslog(self):
        while(True):
            while self.logQueue.qsize() > 0 :
                try:
                    logs = self.logQueue.get()
                except:
                    pass
                else:
                    print(logs)        
                    self.logQueue.task_done()               
                time.sleep(1)
                
    def run(self):
        print(" MyProcessPool run !")
        self.starttime = time.time()
        self.logthread.start()
        
        if self.MaxProcess == 1:
            processfunc(self.task,self.sharedQueue,self.globallock,self.logQueue)
        else:            
            for sprocess in self.processlist:   sprocess.start()
            self._waitFinish()
            
        time.sleep(2)            
        self.endtime = time.time()        
        print(" time cost is %0.3f " %(self.endtime - self.starttime))
            
    def _waitFinish(self):
        for sprocess in self.processlist:  
            sprocess.join()
            sprocess.terminate()
    
        
    def getCoreCount(self):
        coreCount = 1
        try:
            coreCount = multiprocessing.cpu_count()
        except:
            print(" Get core count failed!")
        finally:
            print(" core count is %d " %coreCount)
        
        return coreCount
    
