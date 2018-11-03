# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += database
database_DEPEND_DIRS = configure

DIRS += manyDBRrecordTypesClient
manyDBRrecordTypesClient_DEPEND_DIRS = configure

DIRS += iocBoot
iocBoot_DEPEND_DIRS = configure

include $(TOP)/configure/RULES_TOP


