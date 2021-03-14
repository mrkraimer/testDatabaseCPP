< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/testSpecial.dbd")
testSpecial_registerRecordDeviceDriver(pdbbase)

## Load record instance
dbLoadRecords("db/ai.db","name=exampleLinkAI");
cd ${TOP}/iocBoot/${IOC}
asSetFilename(asconfig)
iocInit()
pvdbcrScalar booleanASL0 boolean
pvdbcrScalar booleanASL1 boolean 1
pvdbcrScalar byteASL0 byte
pvdbcrScalar byteASL1 byte 1
pvdbcrScalar shortASL0 short
pvdbcrScalar shortASL1 short 1
pvdbcrScalar intASL0 int
pvdbcrScalar intASL1 int 1
pvdbcrScalar longASL0 long
pvdbcrScalar longASL1 long 1
pvdbcrScalar ubyteASL0 ubyte
pvdbcrScalar ubyteASL1 ubyte 1
pvdbcrScalar ushortASL0 ushort
pvdbcrScalar ushortASL1 ushort 1
pvdbcrScalar uintASL0 uint
pvdbcrScalar uintASL1 uint 1
pvdbcrScalar ulongASL0 ulong
pvdbcrScalar ulongASL1 ulong 1
pvdbcrScalar floatASL0 float
pvdbcrScalar floatASL1 float 1
pvdbcrScalar doubleASL0 double
pvdbcrScalar doubleASL1 double 1
pvdbcrScalar stringASL0 string
pvdbcrScalar stringASL1 string 1

pvdbcrScalarArray booleanArrayASL0 boolean
pvdbcrScalarArray booleanArrayASL1 boolean 1
pvdbcrScalarArray byteArrayASL0 byte
pvdbcrScalarArray byteArrayASL1 byte 1
pvdbcrScalarArray shortArrayASL0 short
pvdbcrScalarArray shortArrayASL1 short 1
pvdbcrScalarArray intArrayASL0 int
pvdbcrScalarArray intArrayASL1 int 1
pvdbcrScalarArray longArrayASL0 long
pvdbcrScalarArray longArrayASL1 long 1
pvdbcrScalarArray ubyteArrayASL0 ubyte
pvdbcrScalarArray ubyteArrayASL1 ubyte 1
pvdbcrScalarArray ushortArrayASL0 ushort
pvdbcrScalarArray ushortArrayASL1 ushort 1
pvdbcrScalarArray uintArrayASL0 uint
pvdbcrScalarArray uintArrayASL1 uint 1
pvdbcrScalarArray ulongArrayASL0 ulong
pvdbcrScalarArray ulongArrayASL1 ulong 1
pvdbcrScalarArray floatArrayASL0 float
pvdbcrScalarArray floatArrayASL1 float 1
pvdbcrScalarArray doubleArrayASL0 double
pvdbcrScalarArray doubleArrayASL1 double 1
pvdbcrScalarArray stringArrayASL0 string
pvdbcrScalarArray stringArrayASL1 string 1

pvdbcrAddRecord AddRecordLevel0
pvdbcrAddRecord AddRecordLevel1 1
pvdbcrRemoveRecord RemoveRecordLevel0
pvdbcrRemoveRecord RemoveRecordLevel1 1
pvdbcrTraceRecord TraceRecordLevel0
pvdbcrTraceRecord TraceRecordLevel1 1
pvdbcrProcessRecord ProcessRecordLevel0 1
pvdbcrProcessRecord ProcessRecordLevel1 1 1
pvdbcrTestAddRecord

