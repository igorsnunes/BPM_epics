#!../../bin/linux-x86_64/BPM

## You may have to change BPM to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/BPM.dbd"
BPM_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/LPC.db","user=igorHost,number=0")
dbLoadRecords("db/LPC.db","user=igorHost,number=1")
dbLoadRecords("db/LPC.db","user=igorHost,number=2")
dbLoadRecords("db/LPC.db","user=igorHost,number=3")
dbLoadRecords("db/LPC.db","user=igorHost,number=4")
##dbLoadRecords("db/LPC.db","user=igorHost,number=5")
##dbLoadRecords("db/LPC.db","user=igorHost,number=6")
##dbLoadRecords("db/LPC.db","user=igorHost,number=7")
##dbLoadRecords("db/LPC.db","user=igorHost,number=8")
##dbLoadRecords("db/LPC.db","user=igorHost,number=9")
##dbLoadRecords("db/LPC.db","user=igorHost,number=10")
##dbLoadRecords("db/LPC.db","user=igorHost,number=11")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=igorHost"
