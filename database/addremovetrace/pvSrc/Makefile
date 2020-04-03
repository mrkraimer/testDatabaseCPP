TOP=../../..

include $(TOP)/configure/CONFIG

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvDatabase pvAccess pvAccessCA pvData ca Com

INC += pv/addremovetrace.h

DBD += addremovetraceRegister.dbd

LIBRARY = addremovetrace
LIBSRCS += addremovetrace.cpp
LIBSRCS += addremovetraceRegister.cpp
caLink_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

include $(TOP)/configure/RULES

