TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pva2ca pvData ca Com
EPICS_BASE_PVA_CORE_LIBS = pvaClient pvAccess pvAccessCA pvData ca Com

PROD_HOST += timeManyChannel
timeManyChannel_SRCS += timeManyChannel.cpp
timeManyChannel_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += timeManyChannelGPM
timeManyChannelGPM_SRCS += timeManyChannelGPM.cpp
timeManyChannelGPM_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += timeManyChannelGet
timeManyChannelGet_SRCS += timeManyChannelGet.cpp
timeManyChannelGet_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += timeManyChannelPut
timeManyChannelPut_SRCS += timeManyChannelPut.cpp
timeManyChannelPut_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += timeManyChannelMonitor
timeManyChannelMonitor_SRCS += timeManyChannelMonitor.cpp
timeManyChannelMonitor_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += monitormany
monitormany_SRCS += monitormany.cpp
monitormany_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += getmany
getmany_SRCS += getmany.cpp
getmany_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)

PROD_HOST += putmany
putmany_SRCS += putmany.cpp
putmany_LIBS += $(EPICS_BASE_PVA_CORE_LIBS)


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

