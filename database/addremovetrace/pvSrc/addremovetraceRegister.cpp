/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.07.24
 */


/* Author: Marty Kraimer */

#include <cstddef>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <cstdio>
#include <memory>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <pv/pvAccess.h>
#include <pv/pvDatabase.h>

#include <epicsExport.h>
#define epicsExportSharedSymbols
#include "pv/addremovetrace.h"

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace std;
using namespace epics::testDatabase;

static const iocshFuncDef addremovetraceFuncDef = {"addremovetrace", 0};

static void addremovetraceCallFunc(const iocshArgBuf *args)
{
    addremovetrace::create();
}

static void addremovetraceRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&addremovetraceFuncDef, addremovetraceCallFunc);
    }
}


extern "C" {
    epicsExportRegistrar(addremovetraceRegister);
}
