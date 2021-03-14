TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library
#

#=============================
# build an ioc application


DBD += testSpecial.dbd

PROD_IOC += testSpecial


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
testSpecial_SRCS += testSpecial_registerRecordDeviceDriver.cpp
testSpecial_SRCS_DEFAULT += testSpecialMain.cpp
testSpecial_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
testSpecial_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

testSpecial_LIBS += testSpecial
testSpecial_LIBS += pvDatabase pvaClient qsrv pvAccessIOC pvAccess pvAccessCA nt pvData
testSpecial_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

