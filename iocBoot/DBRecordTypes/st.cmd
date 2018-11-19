< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/recordTypes.dbd")
recordTypes_registerRecordDeviceDriver(pdbbase)

## Load record instances
dbLoadRecords("db/stringArray.db","name=DBRstringArray")
dbLoadRecords("db/array.db","name=DBRint8Array,type=CHAR,nelm=100")
dbLoadRecords("db/array.db","name=DBRint16Array,type=SHORT,nelm=100")
dbLoadRecords("db/array.db","name=DBRint32Array,type=LONG,nelm=100")
dbLoadRecords("db/array.db","name=DBRint64Array,type=INT64,nelm=100")
dbLoadRecords("db/array.db","name=DBRuint8Array,type=UCHAR,nelm=100")
dbLoadRecords("db/array.db","name=DBRuint16Array,type=USHORT,nelm=100")
dbLoadRecords("db/array.db","name=DBRuint32Array,type=ULONG,nelm=100")
dbLoadRecords("db/array.db","name=DBRuint64Array,type=UINT64,nelm=100")
dbLoadRecords("db/array.db","name=DBRfloatArray,type=FLOAT,nelm=100")
dbLoadRecords("db/array.db","name=DBRdoubleArray,type=DOUBLE,nelm=100")

dbLoadRecords("db/calc.db","name=DBRcalc")
dbLoadRecords("db/string.db","name=DBRstring")
dbLoadRecords("db/int8.db","name=DBRint8")
dbLoadRecords("db/int16.db","name=DBRint16")
dbLoadRecords("db/int32.db","name=DBRint32")
dbLoadRecords("db/int64.db","name=DBRint64")
dbLoadRecords("db/uint8.db","name=DBRuint8")
dbLoadRecords("db/uint16.db","name=DBRuint16")
dbLoadRecords("db/uint32.db","name=DBRuint32")
dbLoadRecords("db/int64.db","name=DBRint64")
dbLoadRecords("db/uint64.db","name=DBRuint64")
dbLoadRecords("db/double.db","name=DBRdoubleout")
dbLoadRecords("db/float.db","name=DBRfloatout")

dbLoadRecords("db/enum.db","name=DBRenum")

dbLoadRecords("db/simpleBusy.db","name=DBRbusy")
dbLoadRecords("db/longString.db","name=DBRlongstring")

cd ${TOP}/iocBoot/${IOC}
iocInit()
