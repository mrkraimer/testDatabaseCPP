TOP=../../..

include $(TOP)/configure/CONFIG

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvDatabase pvAccess pvAccessCA pvData ca Com

INC += pv/getLinkRecord.h
INC += pv/putLinkRecord.h
INC += pv/monitorLinkRecord.h

DBD += monitorLinkRegister.dbd
DBD += getLinkRegister.dbd
DBD += putLinkRegister.dbd


LIBRARY = pvLink
LIBSRCS += getLinkRecord.cpp
LIBSRCS += putLinkRecord.cpp
LIBSRCS += monitorLinkRecord.cpp
LIBSRCS += getLinkRegister.cpp
LIBSRCS += putLinkRegister.cpp
LIBSRCS += monitorLinkRegister.cpp
caLink_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

include $(TOP)/configure/RULES

