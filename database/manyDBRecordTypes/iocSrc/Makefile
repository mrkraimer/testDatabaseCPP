TOP=../../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#
LIBRARY_IOC += DBRecordTypes

DBD += manyDBRecordTypes.dbd
DBDINC += simpleBusyRecord
DBDINC += charoutRecord
DBDINC += shortoutRecord
DBDINC += ucharoutRecord
DBDINC += ushortoutRecord
DBDINC += ulongoutRecord
DBDINC += floatoutRecord
DBDINC += uint64Record

DBRecordTypes_SRCS+= simpleBusyRecord.c
DBRecordTypes_SRCS+= charoutRecord.c
DBRecordTypes_SRCS+= shortoutRecord.c
DBRecordTypes_SRCS+= ucharoutRecord.c
DBRecordTypes_SRCS+= ushortoutRecord.c
DBRecordTypes_SRCS+= ulongoutRecord.c
DBRecordTypes_SRCS+= floatoutRecord.c
DBRecordTypes_SRCS+= uint64Record.c
example_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an ioc application

PROD_IOC += manyDBRecordTypes


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
manyDBRecordTypes_SRCS += manyDBRecordTypes_registerRecordDeviceDriver.cpp
manyDBRecordTypes_SRCS_DEFAULT += manyDBRecordTypesMain.cpp
manyDBRecordTypes_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
manyDBRecordTypes_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

manyDBRecordTypes_LIBS += DBRecordTypes
manyDBRecordTypes_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData
manyDBRecordTypes_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

