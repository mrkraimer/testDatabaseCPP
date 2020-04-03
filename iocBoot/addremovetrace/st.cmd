< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/addremovetrace.dbd")
addremovetrace_registerRecordDeviceDriver(pdbbase)

## Load record instances

cd ${TOP}/iocBoot/${IOC}
iocInit()
addremovetrace
refshow
