# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += database
database_DEPEND_DIRS = configure

DIRS += DBRrecordTypes
DBRrecordTypes_DEPEND_DIRS = configure

DIRS += timeManyChannels
timeManyChannels_DEPEND_DIRS = configure

DIRS += fastcalc
fastcalc_DEPEND_DIRS = configure

DIRS += testUnion
fastcalc_DEPEND_DIRS = configure

DIRS += iocBoot
iocBoot_DEPEND_DIRS = configure

DIRS += exampleLink

include $(TOP)/configure/RULES_TOP


