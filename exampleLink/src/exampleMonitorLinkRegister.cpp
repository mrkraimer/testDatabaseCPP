/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */


/* Author: Marty Kraimer */

#include <iocsh.h>
#include <pv/channelProviderLocal.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>

// The following must be the last include for code exampleLink uses
#include <epicsExport.h>
#define epicsExportSharedSymbols
#include "pv/exampleMonitorLinkRecord.h"

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
static const iocshArg testArg1 = { "exampleMonitorLinkRecordName", iocshArgString };
static const iocshArg testArg2 = { "linkedRecordName", iocshArgString };
static const iocshArg testArg3 = { "providerNames", iocshArgString };
static const iocshArg *testArgs[] = {
    &testArg0,&testArg1,&testArg2,&testArg3};

static const iocshFuncDef exampleMonitorLinkFuncDef = {
    "exampleMonitorLinkCreateRecord", 4, testArgs};
static void exampleMonitorLinkCallFunc(const iocshArgBuf *args)
{
    string provider("pva");
    string exampleMonitorLinkRecord("exampleMonitorLink");
    string linkedRecordName("doubleArray");
    char *sval = args[0].sval;
    if(sval) provider = string(sval);
    string providerNames(provider);
    sval = args[1].sval;
    if(sval) exampleMonitorLinkRecord = string(sval);
    sval = args[2].sval;
    if(sval) linkedRecordName = string(sval);
    sval = args[3].sval;
    if(sval) providerNames = string(sval);
    PVDatabasePtr master = PVDatabase::getMaster();
    bool result(false);
    PvaClientPtr pva= PvaClient::get(providerNames);
    ExampleMonitorLinkRecordPtr record = ExampleMonitorLinkRecord::create(pva,exampleMonitorLinkRecord,provider,linkedRecordName);
    if(record) 
        result = master->addRecord(record);
    if(!result) cout << "recordname" << " not added" << endl;
}

static void exampleMonitorLinkRegister(void)
{
    static int firstTime = 1;
    if (firstTime) {
        firstTime = 0;
        iocshRegister(&exampleMonitorLinkFuncDef, exampleMonitorLinkCallFunc);
    }
}

extern "C" {
    epicsExportRegistrar(exampleMonitorLinkRegister);
} 
