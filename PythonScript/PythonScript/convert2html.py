

class out2hmtl:
    def __init__(self,outputfile ,template = ".\\htmltemplates\\GOP1table.html",classnamelist = ["better","near","worse"]):
        self.template = template
        self.outputfile = outputfile
        self.tempstr = ""
        self.classnamelist = classnamelist
    
    def _chooseclass(self,value):
        if value > 0.01:
            return self.classnamelist[0]
        elif -0.01 < value < 0.01:
            return self.classnamelist[1]
        else:
            return self.classnamelist[2]
    
    def addrow(self,seq,ratio,fpsratio):
        self.tempstr += "\t\t\t\t<tr>\n"
        self.tempstr += "\t\t\t\t\t<td>" + seq + "<\td>"
        self.tempstr += ("\t\t\t\t\t<td class ='" + self._chooseclass(ratio) + "'>" + str(ratio) + "</td>\n")
        self.tempstr += ("\t\t\t\t\t<td class ='" + self._chooseclass(fpsratio) + "'>" + str(fpsratio) + "</td>\n")         
        self.tempstr += "\t\t\t\t</tr>" 
    
    def output(self):
        templatestr = ""
        with open(self.template) as f:
            templatestr = f.read()
        
        output = templatestr.format(table = self.tempstr)
        with open(self.outputfile,"w") as o:
            o.write(output)
            