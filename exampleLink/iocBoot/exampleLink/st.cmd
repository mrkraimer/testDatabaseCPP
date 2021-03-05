< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/exampleLink.dbd")
exampleLink_registerRecordDeviceDriver(pdbbase)

## Load record instance
dbLoadRecords("db/ai.db","name=exampleLinkAI");

cd ${TOP}/iocBoot/${IOC}
iocInit()
pvDatabaseAccessSecurityCreate(asdefs,null)
doubleArrayCreateRecord
exampleMonitorLinkCreateRecord
exampleGetLinkCreateRecord
examplePutLinkCreateRecord
