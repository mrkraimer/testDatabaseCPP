< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/caLink.dbd")
caLink_registerRecordDeviceDriver(pdbbase)

## Load record instance
dbLoadRecords("db/double.db","name=DBRdouble");
dbLoadRecords("db/inLink.db","name=DBRinLink,inname=DBRdouble");
dbLoadRecords("db/outLink.db","name=DBRoutLink,outname=DBRdouble");

cd ${TOP}/iocBoot/${IOC}
iocInit()
monitorLinkCreateRecord ca PVRMonitorLink DBRdouble
getLinkCreateRecord ca PVRGetLink DBRdouble
putLinkCreateRecord ca PVRPutLink DBRdouble
