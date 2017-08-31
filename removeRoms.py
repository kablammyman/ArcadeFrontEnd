import sys
import os
import shutil

#infile, outfile = sys.argv[1], sys.argv[2]


deleteFolderName = "deleteMe"
if not os.path.exists(deleteFolderName):
	os.mkdir(deleteFolderName)

curPath = os.getcwd()
garbageCan = curPath +"/"+deleteFolderName

romsListFile = open("romsToDelete.txt", "r")
movedFiles = 0
for lines in romsListFile:
	line = lines.split(',')
	filename = line[1]
	filename = filename[:-1]
	romFileName = curPath+"\\"+filename
	romNewHome = garbageCan+"\\"+filename
	#print romFileName
	#print romNewHome
	if os.path.exists(romFileName):
		shutil.move(romFileName, romNewHome)
		movedFiles +=1

romsListFile.close()
print "moved " +str(movedFiles) + " files"