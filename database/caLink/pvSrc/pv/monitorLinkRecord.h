/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2016.06.17
 */
#ifndef EXAMPLEMONITORLINKRECORD_H
#define EXAMPLEMONITORLINKRECORD_H

#include <pv/timeStamp.h>
#include <pv/pvTimeStamp.h>
#include <pv/alarm.h>
#include <pv/pvAlarm.h>
#include <pv/pvDatabase.h>
#include <pv/pvaClient.h>


#include <shareLib.h>

namespace epics { namespace exampleCPP { namespace exampleLink {


class MonitorLinkRecord;
typedef std::tr1::shared_ptr<MonitorLinkRecord> MonitorLinkRecordPtr;
typedef std::tr1::weak_ptr<MonitorLinkRecord> MonitorLinkRecordWPtr;
class MonitorLinkRecordRequester;
typedef std::tr1::shared_ptr<MonitorLinkRecordRequester> MonitorLinkRecordRequesterPtr;


class epicsShareClass MonitorLinkRecord :
    public epics::pvDatabase::PVRecord
{
public:
    POINTER_DEFINITIONS(MonitorLinkRecord);
    static MonitorLinkRecordPtr create(
        epics::pvaClient::PvaClientPtr const &pva,
        std::string const & recordName,
        std::string const & providerName,
        std::string const & channelName
        );
    virtual ~MonitorLinkRecord() {}
    virtual void process();

    virtual bool init() {return false;}
    bool init(
        epics::pvaClient::PvaClientPtr const & pva,
        std::string const & channelName,
        std::string const & providerName
        );
private:
    MonitorLinkRecord(
        std::string const & recordName,
        epics::pvData::PVStructurePtr const & pvStructure);
    bool channelConnected;
    bool isMonitorConnected;
    epics::pvData::PVDoublePtr pvValue;
    epics::pvData::PVStructurePtr pvAlarmField;
    epics::pvData::PVAlarm pvAlarm;
    epics::pvData::Alarm alarm;
    epics::pvData::PVAlarm linkPVAlarm;
    epics::pvData::Alarm linkAlarm;
    epics::pvaClient::PvaClientChannelPtr pvaClientChannel;
    MonitorLinkRecordRequesterPtr linkRecordRequester;
    epics::pvaClient::PvaClientMonitorPtr pvaClientMonitor;
public:
    void channelStateChange(
         epics::pvaClient::PvaClientChannelPtr const & channel,
         bool isConnected);
    void monitorConnect(
        const epics::pvData::Status& status,
        epics::pvaClient::PvaClientMonitorPtr const & monitor,
        epics::pvData::StructureConstPtr const & structure);
    void event(epics::pvaClient::PvaClientMonitorPtr const & monitor);
};

}}}

#endif  /* EXAMPLEMONITORLINKRECORD_H */
