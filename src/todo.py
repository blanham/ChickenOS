#!/usr/bin/python
import os
import re

def grep(path, regex):
	rg = re.compile(regex)
	res = []
	tag = 0
	comment = ""
	linenum = 0
	for root, dir, fnames in os.walk(path):
		for fname in fnames:
			filename = os.path.join(root,fname)
			if(filename.find("cscope.out") != -1):
				continue
			if(filename.find("tags") != -1):
				continue
			if(filename.find("types_c.taghl") != -1):
				continue
			if(filename.find("font") != -1):
				continue
			if(filename.find("img") != -1):
				continue
			if(filename[-4:-1] == ".sw"):
				continue
			if(filename[-4:] == ".vim"):
				continue
			if(filename[-2:] == ".o"):
				continue
			if(filename[-3:] == ".py"):
				continue
			if os.path.isfile(filename):
				with open(filename) as f:
					linenum = 0
					for line in f:
						linenum += 1
						if rg.search(line):
							tag = 6
							res.append(filename + "\n")
							line = line.strip()
							if(line.find("/*") != -1):
								comment = "*/"
							res.append(str(linenum) + ": " + line + "\n")
						elif tag > 1:
							line = line.strip()
							if(line.find("/*") != -1):
								comment = "*/"
							elif(line.find("*/") != -1):
								comment = ""

							res.append(str(linenum) + ": " + line + "\n")

							tag-=1
						elif tag == 1:
							tag = 0
							res.append(comment + "\n")
							comment = ""
							
							
	return res
def main():
	result = grep('.',"TODO")
	result += grep('.',"XXX")
	result += grep('.',"FIXME")
	#for i in result:
	print "".join(result)
	print "/* ex: set syntax=c: */"
if __name__ == "__main__":
	main();
