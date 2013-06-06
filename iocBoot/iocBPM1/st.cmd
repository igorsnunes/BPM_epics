#!../../bin/linux-x86_64/BPM

## You may have to change BPM to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/BPM.dbd"
BPM_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/LPC.db","user=igorHost")

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=igorHost"
