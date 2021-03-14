/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2021.03.13
 */
#ifndef PVDBCRTESTADDRECORD_H
#define PVDBCRTESTADDRECORD_H

#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>


#include <shareLib.h>

namespace epics { namespace testSpecial {


class PvdbcrTestAddRecord;
typedef std::tr1::shared_ptr<PvdbcrTestAddRecord> PvdbcrTestAddRecordPtr;
typedef std::tr1::weak_ptr<PvdbcrTestAddRecord> PvdbcrTestAddRecordWPtr;


class epicsShareClass PvdbcrTestAddRecord :
    public epics::pvDatabase::PVRecord
{
private:
    PvdbcrTestAddRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);      
    epics::pvData::PVStringPtr pvLink;
    epics::pvData::PVStringPtr pvProto;
    epics::pvData::PVStringPtr pvNew;
    epics::pvData::PVStringPtr pvAccessMethod;
    epics::pvData::PVStructurePtr pvAlarmField;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
    void clientProcess();
    void databaseProcess();
public:
    POINTER_DEFINITIONS(PvdbcrTestAddRecord);
    static PvdbcrTestAddRecordPtr create(std::string const & recordName);
    virtual ~PvdbcrTestAddRecord() {}
    virtual void process();
    virtual bool init();
};

}}

#endif  /* PVDBCRTESTADDRECORD_H */
