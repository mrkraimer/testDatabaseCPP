/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.07.24
 */

/* Author: Marty Kraimer */
#include <pv/pvDatabase.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>

#include <pv/channelProviderLocal.h>
#include <pv/traceRecord.h>
#include <pv/removeRecord.h>
#include <pv/addRecord.h>

#define epicsExportSharedSymbols
#include "pv/addremovetrace.h"

using namespace std;
using std::tr1::static_pointer_cast;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace epics::testDatabase;

static FieldCreatePtr fieldCreate = getFieldCreate();
static StandardFieldPtr standardField = getStandardField();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();
static StandardPVFieldPtr standardPVField = getStandardPVField();

void addremovetrace::create()
{
    PVDatabasePtr master = PVDatabase::getMaster();
    bool result(false);
    
    result = master->addRecord(TraceRecord::create("PVRtraceRecord"));
    if(!result) cout<< "record PVRtraceRecord not added\n";
    result = master->addRecord(RemoveRecord::create("PVRremoveRecord"));
    if(!result) cout<< "record PVRremoveRecord not added\n";
    result = master->addRecord(AddRecord::create("PVRaddRecord"));
    if(!result) cout<< "record PVRaddRecord not added\n";

}

