/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2021.03.13
 */

#include <pv/standardPVField.h>
#include <pv/standardField.h>
#include <pv/standardPVField.h>
#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>
#include <pv/convert.h>

#define epicsExportSharedSymbols
#include "pv/pvdbcrTestAddRecord.h"

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvDatabase;
using namespace epics::pvaClient;
using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using std::cout;
using std::endl;
using std::string;

namespace epics { namespace testSpecial { 

static FieldCreatePtr fieldCreate = getFieldCreate();
static StandardFieldPtr standardField = getStandardField();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();

PvdbcrTestAddRecordPtr PvdbcrTestAddRecord::create(std::string const & recordName)
{
    StructureConstPtr top = fieldCreate->createFieldBuilder()->
        add("linkRecord",pvString) ->
        add("protoRecord",pvString) ->
        add("newRecord",pvString) ->
        add("accessMethod",pvString) ->
        add("timeStamp",standardField->timeStamp()) ->
        add("alarm",standardField->alarm()) ->
        createStructure();
    PVStructurePtr pvStructure = pvDataCreate->createPVStructure(top);
    PvdbcrTestAddRecordPtr pvRecord(
        new PvdbcrTestAddRecord(recordName,pvStructure)); 
    if(!pvRecord->init()) pvRecord.reset();   
    return pvRecord;
}

PvdbcrTestAddRecord::PvdbcrTestAddRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure)
{
}


bool PvdbcrTestAddRecord::init()
{
    initPVRecord();
    PVStructurePtr pvStructure = getPVRecordStructure()->getPVStructure();
    pvLink = pvStructure->getSubField<PVString>("linkRecord");
    pvProto = pvStructure->getSubField<PVString>("protoRecord");
    pvNew = pvStructure->getSubField<PVString>("newRecord");
    pvAccessMethod = pvStructure->getSubField<PVString>("accessMethod");
    pvAccessMethod->put(std::string("client"));
    pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
    pvAlarm.attach(pvAlarmField);
    return true;
}

void PvdbcrTestAddRecord::process()
{
   std::string accessMethod = pvAccessMethod->get();
   if(accessMethod.compare("client")==0) {
        clientProcess();
        return;
   }
   if(accessMethod.compare("database")==0) {
        databaseProcess();
        return;
   }
   throw std::runtime_error("illegal accessMethod: must be client or database");
}

void PvdbcrTestAddRecord::clientProcess()
{
    PvaClientPtr pva = PvaClient::get("pva");
    PvaClientChannelPtr linkChannel = pva->channel(pvLink->get());
    PvaClientPutGetPtr putGet = linkChannel->createPutGet();
    putGet->connect();
    PvaClientPutDataPtr putData = putGet->getPutData();
    PVStructurePtr arg = putData->getPVStructure();
    PvaClientChannelPtr protoChannel = pva->channel(pvProto->get());
    PvaClientGetPtr protoGet = protoChannel->createGet();
    protoGet->connect();
    protoGet->get();
    PVStructurePtr pvValue = protoGet->getData()->getPVStructure();
    PVStringPtr pvRecordName = arg->getSubField<PVString>("argument.recordName");
    PVUnionPtr pvUnion = arg->getSubField<PVUnion>("argument.union");
    pvRecordName->put(pvNew->get());
    pvUnion->set(pvValue);
    putGet->putGet();
    PvaClientGetDataPtr getData = putGet->getGetData();
    PVStringPtr pvStatus = getData->getPVStructure()->getSubField<PVString>("result.status");
    alarm.setMessage(pvStatus->get());
    pvAlarm.set(alarm);
    PVRecord::process();
}

void PvdbcrTestAddRecord::databaseProcess()
{
    PVDatabasePtr pvDatabase = PVDatabase::getMaster();
    PVRecordPtr pvLinkRecord = pvDatabase->findRecord(pvLink->get());
    if(!pvLinkRecord) {
        alarm.setMessage(string("record ") + pvLink->get() + string(" does not exist"));
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
        PVRecord::process();
        return;
    }
    PVRecordPtr pvProtoRecord = pvDatabase->findRecord(pvProto->get());
    if(!pvProtoRecord) {
        alarm.setMessage(string("record ") + pvProto->get() + string(" does not exist"));
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
        PVRecord::process();
        return;
    }
    PVStringPtr pvRecordName = pvLinkRecord->getPVStructure()->getSubField<PVString>("argument.recordName");
    if(!pvRecordName) {
        alarm.setMessage(string("record ") + pvLink->get()
            + string(" argument.recordName is not a string field"));
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
        PVRecord::process();
        return;
    }
    PVUnionPtr pvUnion = pvLinkRecord->getPVStructure()->getSubField<PVUnion>("argument.union");
    if(!pvUnion) {
        alarm.setMessage(string("record ") + pvLink->get() + string(" argument.value is not a union field"));
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
        PVRecord::process();
        return;
    }
    PVStringPtr pvStatus = pvLinkRecord->getPVStructure()->getSubField<PVString>("result.status");
    if(!pvStatus) {
        alarm.setMessage(string("record ") + pvLink->get()
            + string(" result.status is not a string field"));
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
        PVRecord::process();
        return;
    }
    PVStructurePtr pvValue = pvProtoRecord->getPVStructure();
    try {
        epicsGuard <PVRecord> guard(*pvLinkRecord);
        pvLinkRecord->beginGroupPut();
        pvRecordName->put(pvNew->get());
        pvUnion->set(pvValue);
        pvLinkRecord->process();
        pvLinkRecord->endGroupPut();
    } catch(std::exception& ex) {
        Status status = Status(Status::STATUSTYPE_FATAL, ex.what());
        alarm.setMessage(status.getMessage());
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
    }
    alarm.setMessage(pvStatus->get());
    alarm.setSeverity(noAlarm);
    pvAlarm.set(alarm);
    PVRecord::process();
}

}}
