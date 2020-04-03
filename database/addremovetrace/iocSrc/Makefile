TOP=../../..

include $(TOP)/configure/CONFIG

#=============================
# build an ioc application

DBD += addremovetrace.dbd

PROD_IOC += addremovetrace


# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
addremovetrace_SRCS += addremovetrace_registerRecordDeviceDriver.cpp
addremovetrace_SRCS_DEFAULT += addremovetraceMain.cpp
addremovetrace_SRCS_vxWorks += -nil-


# The following adds support from base/src/vxWorks
addremovetrace_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

addremovetrace_LIBS += addremovetrace
addremovetrace_LIBS += pvDatabase qsrv pvaClient pvAccessIOC pvAccess pvAccessCA nt pvData

addremovetrace_LIBS += $(EPICS_BASE_IOC_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32

include $(TOP)/configure/RULES

