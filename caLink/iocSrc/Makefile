TOP=../..

include $(TOP)/configure/CONFIG

#=============================
# build an ioc application

DBD += caLink.dbd

PROD_IOC += caLink


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
caLink_SRCS += caLink_registerRecordDeviceDriver.cpp
caLink_SRCS_DEFAULT += caLinkMain.cpp
caLink_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
caLink_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

caLink_LIBS += pvLink
caLink_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData

caLink_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

include $(TOP)/configure/RULES

