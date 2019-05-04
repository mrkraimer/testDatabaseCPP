/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.04.02
 */
#ifndef SOFTRECORD_H
#define SOFTRECORD_H


#include <pv/timeStamp.h>
#include <pv/alarm.h>
#include <pv/pvTimeStamp.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>


#include <shareLib.h>


namespace epics { namespace exampleCPP {namespace softRecord { 

class SoftRecord;
typedef std::tr1::shared_ptr<SoftRecord> SoftRecordPtr;

class epicsShareClass SoftRecord :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(SoftRecord);
    static SoftRecordPtr create(
        std::string const & recordName);
    virtual ~SoftRecord() {}
    virtual bool init() {return false;}
    virtual void process();
    
private:
    SoftRecord(std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    void initPvt();

    epics::pvData::PVDoublePtr pvCurrent;
    epics::pvData::PVDoublePtr pvPower;
    epics::pvData::PVDoublePtr pvVoltage;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
};


}}}

#endif  /* SOFTRECORD_H */
