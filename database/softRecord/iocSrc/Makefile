TOP=../../..

include $(TOP)/configure/CONFIG

#=============================
# build an ioc application

DBD += softRecord.dbd

PROD_IOC += softRecord


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
softRecord_SRCS += softRecord_registerRecordDeviceDriver.cpp
softRecord_SRCS_DEFAULT += softRecordMain.cpp
softRecord_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
softRecord_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

softRecord_LIBS += softRecord
softRecord_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData

softRecord_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

include $(TOP)/configure/RULES

