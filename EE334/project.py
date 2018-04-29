#imports
import time
import os

HEXTABLE={
	"0":0, "1":1, "2":2, "3":3,
	"4":4, "5":5, "6":6, "7":7,
	"8":8, "9":9, "a":10, "b":11,
	"c":12, "d":13, "e":14, "f":15
}

FD=open("trace_sample.txt","r+")
branchFlag = 0
#data struct
class Data():
	#easier to make it when needed
	def __init__(self, setAddress=0, setTarget_address=0, setPrediction=0, setBusy=0):
		self.address=setAddress
		self.target_address=setTarget_address
		self.prediction=setPrediction
		self.busy=setBusy
		self.timeStamp=time.time()

	def updateTime(self):
		self.timeStamp=time.time()

	#vars needed
	address=0
	target_address=0
	prediction=0
	busy=0
	timeStamp=0

#Branch target buffer
BTB=[None]*1024

def getIndex(inputStr=""):
	inputStr=inputStr[3:]
	firstInt=HEXTABLE[inputStr[0]]*16*16
	secondInt=HEXTABLE[inputStr[1]]*16
	thirdInt=HEXTABLE[inputStr[2]]

	return (firstInt+secondInt+thirdInt)/4

def covertPC(inputStr=""):
	inputStr="0x"+inputStr
	return int(inputStr,0)

def checkForUpdate(index, prePC, curPC):
	global branchFlag
	if(branchFlag and BTB[index].prediction < 3 and BTB[index].target_address == curPC):
		BTB[index].updateTime()
		if(BTB[index].prediction == 2):
			BTB[index].prediction -= 1
			
	elif(branchFlag == 0 and BTB[index].prediction < 3):
		BTB[index].prediction += 1
		BTB[index].updateTime()
	elif(branchFlag == 0 and BTB[index].prediction > 2):
		BTB[index].updateTime()
		if(BTB[index].prediction == 3):
			BTB[index].prediction += 1
	elif(branchFlag and BTB[index].prediction < 3 and BTB[index].target_address != curPC):
		BTB[index].updateTime()
		BTB[index].target_address = curPC
		if(BTB[index].prediction == 2):
			BTB[index].prediction -= 1
	elif(branchFlag and BTB[index].prediction > 2):
		BTB[index].updateTime()
		BTB[index].prediction -= 1
		if(BTB[index].prediction == 2):
			BTB[index].target_address = curPC

def printBuffer():
	for x in range(0,1024):
		if not BTB[x] is None:
			print "BTB[%d]" % x
			print BTB[x].address
			print BTB[x].target_address
			print BTB[x].prediction
			print BTB[x].busy
			print BTB[x].timeStamp

def main():
	global branchFlag
	timeVar=0
	for x in FD:
		prePC=x.strip()
		break

	
	for x in FD:
		curPC=x.strip()
		break

	#frist get index and actual
	
	preIndex=getIndex(prePC)
	curIndex=getIndex(curPC)

	acutalPre=covertPC(prePC)
	acutalCur=covertPC(curPC)

	# if ((acutalCur-acutalPre)==4): branchFlag=0
	# else: branchFlag=1

	# if BTB[preIndex]==None:
	# 	if branchFlag:
	# 		BTB[preIndex]=Data(prePC, curPC, 1, 1)
	# elif (BTB[512+preIndex] == None):
	# 	if branchFlag:
	# 		BTB[512+preIndex] = Data(prePC, curPC, 1, 1)
	# elif (BTB[preIndex].address == prePC):
	# 	checkForUpdate(preIndex, prePC, curPC)
	# elif (BTB[512+preIndex].address == prePC):
	# 	checkForUpdate(512+preIndex, prePC, curPC)
	# else:
	# 	if(BTB[preIndex].timeStamp > BTB[512+preIndex].timeStamp):
	# 		if (branchFlag):
	# 			BTB[512+preIndex] = Data(prePC, curPC, 1, 1)
	# 	else:
	# 		if(branchFlag):
	# 			BTB[preIndex] = Data(prePC, curPC, 1, 1)

	if ((acutalCur-acutalPre)==4): branchFlag=0
	else: branchFlag=1

	if BTB[preIndex]==None:
		if branchFlag:
			BTB[preIndex]=Data(prePC, curPC, 1, 1)
	elif (BTB[preIndex].address == prePC):
		checkForUpdate(preIndex, prePC, curPC)
	else:
		if(branchFlag):
			BTB[preIndex] = Data(prePC, curPC, 1, 1)
	


	time.sleep(.01)

	for x in FD:
		prePC=curPC
		curPC=x.strip()



		preIndex=getIndex(prePC)
		curIndex=getIndex(curPC)

		acutalPre=covertPC(prePC)
		acutalCur=covertPC(curPC)

		if ((acutalCur-acutalPre)==4): branchFlag=0
		else: branchFlag=1


		if BTB[preIndex]==None:
			if branchFlag:
				BTB[preIndex]=Data(prePC, curPC, 1, 1)
		elif (BTB[preIndex].address == prePC):
			checkForUpdate(preIndex, prePC, curPC)
		else:
			if(branchFlag):
				BTB[preIndex] = Data(prePC, curPC, 1, 1)

		# if ((acutalCur-acutalPre)==4): branchFlag=0
		# else: branchFlag=1

		# if BTB[preIndex]==None:
		# 	if branchFlag:
		# 		BTB[preIndex]=Data(prePC, curPC, 1, 1)
		# elif (BTB[512+preIndex] == None):
		# 	if branchFlag:
		# 		BTB[512+preIndex] = Data(prePC, curPC, 1, 1)
		# elif (BTB[preIndex].address == prePC):
		# 	checkForUpdate(preIndex, prePC, curPC)
		# elif (BTB[512+preIndex].address == prePC):
		# 	checkForUpdate(512+preIndex, prePC, curPC)
		# else:
		# 	if(BTB[preIndex].timeStamp >= BTB[512+preIndex].timeStamp):
		# 		if (branchFlag):
		# 			BTB[512+preIndex] = Data(prePC, curPC, 1, 1)
		# 	else:
		# 		if(branchFlag):
		# 			BTB[preIndex] = Data(prePC, curPC, 1, 1)




		time.sleep(.0001)
		#os.system("clear")
		#print "Itteration Var: %d" % timeVar
		timeVar+=1
		

	printString="Index:%4d, Address: %s, TargetPC: %s, Prediction:%d, Busy:%d"

	for x in range(0,1024):
		if not BTB[x] is None:
			print printString %(x, BTB[x].address, BTB[x].target_address, BTB[x].prediction, BTB[x].busy)





	

		#if BTB[indexValue]==None:#empty
		#else:#not empty


	pass


if __name__ == '__main__':
	main()