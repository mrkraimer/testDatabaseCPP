TOP=..

include $(TOP)/configure/CONFIG

exampleLinkSRC = $(TOP)/src

EPICS_BASE_PVA_CORE_LIBS = pvDatabase pvaClient pvAccess pvAccessCA nt pvData ca Com

INC += pv/pvdbcrTestAddRecord.h

DBD += pvdbcrTestAddRecordRegister.dbd

LIBRARY = testSpecial

LIBSRCS += pvdbcrTestAddRecord.cpp
LIBSRCS += pvdbcrTestAddRecordRegister.cpp
testSpecial_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_SYS_LIBS_WIN32 += ws2_32


include $(TOP)/configure/RULES


