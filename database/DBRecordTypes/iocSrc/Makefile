TOP=../../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#
LIBRARY_IOC += DBRecordTypes

USR_CFLAGS += -DUSE_TYPED_RSET

DBRecordTypes_SRCS+= simpleBusyRecord.c
DBRecordTypes_SRCS+= charoutRecord.c
DBRecordTypes_SRCS+= shortoutRecord.c
DBRecordTypes_SRCS+= ucharoutRecord.c
DBRecordTypes_SRCS+= ushortoutRecord.c
DBRecordTypes_SRCS+= ulongoutRecord.c
DBRecordTypes_SRCS+= floatoutRecord.c
DBRecordTypes_SRCS+= uint64Record.c
example_LIBS += $(EPICS_BASE_IOC_LIBS)

DBD += recordTypes.dbd
DBDINC += simpleBusyRecord
DBDINC += charoutRecord
DBDINC += shortoutRecord
DBDINC += ucharoutRecord
DBDINC += ushortoutRecord
DBDINC += ulongoutRecord
DBDINC += floatoutRecord
DBDINC += uint64Record

#=============================
# build an ioc application

PROD_IOC += recordTypes


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
recordTypes_SRCS += recordTypes_registerRecordDeviceDriver.cpp
recordTypes_SRCS_DEFAULT += recordTypesMain.cpp
recordTypes_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
recordTypes_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

recordTypes_LIBS += DBRecordTypes
recordTypes_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData
recordTypes_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

