# Makefile at top of application tree

TOP = .
include $(TOP)/configure/CONFIG

DIRS += configure

DIRS += exampleDatabase
exampleDatabase_DEPEND_DIRS = configure
DIRS += caLink
caLink_DEPEND_DIRS = configure
DIRS += manyDBRecords
manyDBRecords_DEPEND_DIRS = configure

DIRS += iocBoot
iocBoot_DEPEND_DIRS = configure

include $(TOP)/configure/RULES_TOP


