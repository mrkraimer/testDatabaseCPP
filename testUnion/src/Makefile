TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pvAccessCA nt pvData ca Com
#EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pva2ca nt pvData ca Com


PROD_HOST += addUnionRecord
addUnionRecord_SRCS += addUnionRecord.cpp
addUnionRecord_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)


PROD_HOST += putUnion
putUnion_SRCS += putUnion.cpp
putUnion_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)


PROD_SYS_LIBS_WIN32 += ws2_32


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

