< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/exampleDatabase.dbd")
exampleDatabase_registerRecordDeviceDriver(pdbbase)

## Load record instances
dbLoadRecords("db/dbStringArray.db","name=DBRstringArray01")
dbLoadRecords("db/dbEnum.db","name=DBRenum01")
dbLoadRecords("db/dbCounter.db","name=DBRcounter01")
dbLoadRecords("db/dbSimpleBusy.db","name=DBRbusy")
dbLoadRecords("db/dbArray.db","name=DBRdoubleArray,type=DOUBLE")
dbLoadRecords("db/test.db")
dbLoadRecords("db/dbLongString.db","name=DBRlongstring")



cd ${TOP}/iocBoot/${IOC}
iocInit()
exampleDatabase
