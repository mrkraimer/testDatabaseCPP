/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */


/* Author: Marty Kraimer */

#include <cstddef>
#include <cstdlib>
#include <cstddef>
#include <string>
#include <cstdio>
#include <memory>
#include <iostream>

#include <cantProceed.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsThread.h>
#include <iocsh.h>

#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/pvAccess.h>
#include <pv/ntscalarArray.h>
#include <pv/pvaClient.h>
#include <pv/pvDatabase.h>

#include <epicsExport.h>
#include <pv/putLinkRecord.h>

using namespace epics::pvData;
using namespace epics::nt;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::pvDatabase;
using namespace epics::exampleCPP::exampleLink;
using std::cout;
using std::endl;
using std::string;

static StandardPVFieldPtr standardPVField = getStandardPVField();

static const iocshArg testArg0 = { "provider", iocshArgString };
static const iocshArg testArg1 = { "putLinkRecordName", iocshArgString };
static const iocshArg testArg2 = { "linkedRecordName", iocshArgString };
static const iocshArg testArg3 = { "providerNames", iocshArgString };
static const iocshArg *testArgs[] = {
    &testArg0,&testArg1,&testArg2,&testArg3};

static const iocshFuncDef putLinkFuncDef = {
    "putLinkCreateRecord", 4, testArgs};
static void putLinkCallFunc(const iocshArgBuf *args)
{
    string provider("pva");
    string putLinkRecord("putLink");
    string linkedRecordName("doubleArray");
    char *sval = args[0].sval;
    if(sval) provider = string(sval);
    string providerNames(provider);
    sval = args[1].sval;
    if(sval) putLinkRecord = string(sval);
    sval = args[2].sval;
    if(sval) linkedRecordName = string(sval);
    sval = args[3].sval;
    if(sval) providerNames = string(sval);
    PVDatabasePtr master = PVDatabase::getMaster();
    bool result(false);
    PvaClientPtr pva= PvaClient::get(providerNames);
    PutLinkRecordPtr record = PutLinkRecord::create(
       pva,putLinkRecord,provider,linkedRecordName);
    if(record) 
        result = master->addRecord(record);
    if(!result) cout << "recordname" << " not added" << endl;
}

static void putLinkRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&putLinkFuncDef, putLinkCallFunc);
    }
}

extern "C" {
    epicsExportRegistrar(putLinkRegister);
} 
