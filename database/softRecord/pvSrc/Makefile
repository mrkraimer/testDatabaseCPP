TOP=../../..

include $(TOP)/configure/CONFIG

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvDatabase pvAccess pvAccessCA pvData ca Com

DBD += softRecordRegister.dbd

LIBRARY = softRecord
LIBSRCS += softRecordRegister.cpp
caLink_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

include $(TOP)/configure/RULES

