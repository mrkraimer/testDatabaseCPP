TOP=../../..

include $(TOP)/configure/CONFIG

#=============================
# build an ioc application

DBD += manyDBRecords.dbd

PROD_IOC += manyDBRecords


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
manyDBRecords_SRCS += manyDBRecords_registerRecordDeviceDriver.cpp
manyDBRecords_SRCS_DEFAULT += manyDBRecordsMain.cpp
manyDBRecords_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
manyDBRecords_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

manyDBRecords_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData

manyDBRecords_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

include $(TOP)/configure/RULES

