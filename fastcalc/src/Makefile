TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pva2ca pvData ca Com
EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pvAccessCA pvData ca Com

PROD_HOST += monitorme
monitorme_SRCS += monitorme.cpp
monitorme_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += monitor
monitor_SRCS += monitor.cpp
monitor_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += monitormerate
monitormerate_SRCS += monitormerate.cpp
monitormerate_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += monitorrate
monitorrate_SRCS += monitorrate.cpp
monitorrate_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += getputmonitorrate
getputmonitorrate_SRCS += getputmonitorrate.cpp
getputmonitorrate_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)



#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

