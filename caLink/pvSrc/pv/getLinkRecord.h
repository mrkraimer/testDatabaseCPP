/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */
#ifndef EXAMPLEGETLINKRECORD_H
#define EXAMPLEGETLINKRECORD_H


#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>

#include <shareLib.h>

namespace epics { namespace exampleCPP { namespace exampleLink {


class GetLinkRecord;
typedef std::tr1::shared_ptr<GetLinkRecord> GetLinkRecordPtr;
typedef std::tr1::weak_ptr<GetLinkRecord> GetLinkRecordWPtr;
class GetLinkRecordRequester;
typedef std::tr1::shared_ptr<GetLinkRecordRequester> GetLinkRecordRequesterPtr;

class epicsShareClass GetLinkRecord :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(GetLinkRecord);
    static GetLinkRecordPtr create(
        epics::pvaClient::PvaClientPtr const &pva,
        std::string const & recordName,
        std::string const & providerName,
        std::string const & channelName
        );
    virtual ~GetLinkRecord() {}
    virtual void process();
    virtual bool init() {return false;}
    bool init(
        epics::pvaClient::PvaClientPtr const & pva,
        std::string const & channelName,
        std::string const & providerName
        );
private:
    GetLinkRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    bool channelConnected;
    bool isGetConnected;
    epics::pvData::PVDoublePtr pvValue;
    epics::pvData::PVStructurePtr pvAlarmField;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
    epics::pvData::PVAlarm linkPVAlarm;
    epics::pvData::Alarm linkAlarm;
    epics::pvaClient::PvaClientChannelPtr pvaClientChannel;
    GetLinkRecordRequesterPtr linkRecordRequester;
    epics::pvaClient::PvaClientGetPtr pvaClientGet;
public:
    void channelStateChange(
         epics::pvaClient::PvaClientChannelPtr const & channel,
         bool isConnected);
    void channelGetConnect(
        const epics::pvData::Status& status,
        epics::pvaClient::PvaClientGetPtr const & clientGet);
};

}}}

#endif  /* EXAMPLEGETLINKRECORD_H */
