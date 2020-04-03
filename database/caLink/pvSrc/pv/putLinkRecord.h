/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */
#ifndef EXAMPLEPUTLINKRECORD_H
#define EXAMPLEPUTLINKRECORD_H

#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>


#include <shareLib.h>

namespace epics { namespace testDatabase {


class PutLinkRecord;
typedef std::tr1::shared_ptr<PutLinkRecord> PutLinkRecordPtr;
typedef std::tr1::weak_ptr<PutLinkRecord> PutLinkRecordWPtr;
class PutLinkRecordRequester;
typedef std::tr1::shared_ptr<PutLinkRecordRequester> PutLinkRecordRequesterPtr;


class epicsShareClass PutLinkRecord :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(PutLinkRecord);
    static PutLinkRecordPtr create(
        epics::pvaClient::PvaClientPtr const &pva,
        std::string const & recordName,
        std::string const & providerName,
        std::string const & channelName
        );
    virtual ~PutLinkRecord() {}
    virtual void process();
    virtual bool init() {return false;}
    bool init(
        epics::pvaClient::PvaClientPtr const & pva,
        std::string const & channelName,
        std::string const & providerName
        );
private:
    PutLinkRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    bool channelConnected;
    bool isPutConnected;
    bool isPutDone;
    bool setAlarmGood;
    epics::pvData::PVDoublePtr pvValue;
    epics::pvData::PVStructurePtr pvAlarmField;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
    epics::pvaClient::PvaClientChannelPtr pvaClientChannel;
    PutLinkRecordRequesterPtr linkRecordRequester;
    epics::pvaClient::PvaClientPutPtr pvaClientPut;
public:
    void channelStateChange(
         epics::pvaClient::PvaClientChannelPtr const & channel,
         bool isConnected);
    void channelPutConnect(
        const epics::pvData::Status& status,
        epics::pvaClient::PvaClientPutPtr const & clientPut);
    void putDone(
        const epics::pvData::Status& status,
        epics::pvaClient::PvaClientPutPtr const & clientPut);
};

}}

#endif  /* EXAMPLEPUTLINKRECORD_H */
