/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.08.02
 */

#include <pv/standardPVField.h>
#include <pv/ntscalar.h>
#include <pv/pvaClient.h>

#include <epicsExport.h>
#include <pv/monitorLinkRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::pvDatabase;
using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using std::cout;
using std::endl;
using std::string;

namespace epics { namespace testDatabase {

class MonitorLinkRecordRequester :
    public PvaClientChannelStateChangeRequester,
    public PvaClientMonitorRequester
{
    MonitorLinkRecordWPtr exampleMonitorLinkRecord;
    PvaClient::weak_pointer pvaClient;
public:
    POINTER_DEFINITIONS(MonitorLinkRecordRequester);

    MonitorLinkRecordRequester(
        MonitorLinkRecordPtr const & exampleMonitorLinkRecord,
        PvaClientPtr const &pvaClient)
    : exampleMonitorLinkRecord(exampleMonitorLinkRecord),
      pvaClient(pvaClient)
    {}
    virtual ~MonitorLinkRecordRequester() {
        if(PvaClient::getDebug()) std::cout << "~MonitorLinkRecordRequester" << std::endl;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        MonitorLinkRecordPtr monitorLinkRecord(exampleMonitorLinkRecord.lock());
        if(!monitorLinkRecord) return;
        monitorLinkRecord->channelStateChange(channel,isConnected);  
    }

    virtual void monitorConnect(
        const Status& status,
        PvaClientMonitorPtr const & monitor,
        StructureConstPtr const & structure)
    {
        MonitorLinkRecordPtr monitorLinkRecord(exampleMonitorLinkRecord.lock());
        if(!monitorLinkRecord) return;
        monitorLinkRecord->monitorConnect(status,monitor,structure);  
    }

    virtual void event(PvaClientMonitorPtr const & monitor)
    {
        MonitorLinkRecordPtr monitorLinkRecord(exampleMonitorLinkRecord.lock());
        if(!monitorLinkRecord) return;
        monitorLinkRecord->event(monitor);  
    }   
};


MonitorLinkRecordPtr MonitorLinkRecord::create(
    PvaClientPtr  const & pva,
    string const & recordName,
    string const & providerName,
    string const & channelName)
{
    PVStructurePtr pvStructure = getStandardPVField()->scalar(
        pvDouble,"timeStamp,alarm");
    MonitorLinkRecordPtr pvRecord(
        new MonitorLinkRecord(recordName,pvStructure)); 
    
    MonitorLinkRecordRequesterPtr linkRecordRequester(
        MonitorLinkRecordRequesterPtr(new MonitorLinkRecordRequester(pvRecord,pva)));
    pvRecord->linkRecordRequester = linkRecordRequester;
    if(!pvRecord->init(pva,channelName,providerName)) pvRecord.reset();
    return pvRecord;
}

MonitorLinkRecord::MonitorLinkRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure),
  channelConnected(false),
  isMonitorConnected(false)
{
}


bool MonitorLinkRecord::init(
    PvaClientPtr const & pvaClient,
    string const & channelName,
    string const & providerName)
{
    initPVRecord();
    PVStructurePtr pvStructure = getPVRecordStructure()->getPVStructure();
    pvValue = pvStructure->getSubField<PVDouble>("value");
    if(!pvValue) {
        throw std::runtime_error("value is not a double");
    }
    pvAlarmField = pvStructure->getSubField<PVStructure>("alarm");
    if(!pvAlarmField) {
        throw std::runtime_error("no alarm field");
    }
    if(!pvAlarm.attach(pvAlarmField)) {
        throw std::runtime_error(string("bad alarm field"));
    }
    alarm.setMessage("never connected");
    alarm.setSeverity(invalidAlarm);
    alarm.setStatus(clientStatus);
    pvAlarm.set(alarm);
    pvaClientChannel = pvaClient->createChannel(channelName,providerName);
    pvaClientChannel->setStateChangeRequester(linkRecordRequester);
    pvaClientChannel->issueConnect();
    return true;
}

void MonitorLinkRecord::process()
{
    if(!channelConnected)
    {
        alarm.setMessage("disconnected");
        alarm.setSeverity(invalidAlarm);
        alarm.setStatus(clientStatus);
        pvAlarm.set(alarm);
    } else if(!isMonitorConnected) 
    {
        alarm.setMessage("monitor not connected");
        alarm.setSeverity(invalidAlarm);
        alarm.setStatus(clientStatus);
        pvAlarm.set(alarm);
    }
    PVRecord::process();
}

void MonitorLinkRecord::channelStateChange(
    PvaClientChannelPtr const & channel, bool isConnected)
{
    channelConnected = isConnected;
    if(isConnected) {
        if(!pvaClientMonitor) {
            pvaClientMonitor = pvaClientChannel->createMonitor("value,alarm");
            pvaClientMonitor->setRequester(linkRecordRequester);
            pvaClientMonitor->issueConnect();
        }
        return;
    }
    lock();
    try {
        beginGroupPut();
        process();
        endGroupPut();
    } catch(...) {
       unlock();
       throw;
    }
    unlock();
}

void MonitorLinkRecord::monitorConnect(
    const Status& status,
    PvaClientMonitorPtr const & monitor,
    StructureConstPtr const & structure)
{
    if(status.isOK()) {
        isMonitorConnected = true;
        return;
    }
    lock();
        try {
            isMonitorConnected = false;
            beginGroupPut();
            process();
            endGroupPut();
        } catch(...) {
           unlock();
           throw;
        }
    unlock();
}

void MonitorLinkRecord::event(PvaClientMonitorPtr const & monitor)
{
    while(monitor->poll()) {
        PVStructurePtr pvStructure = monitor->getData()->getPVStructure();
        PVDoublePtr pvDouble = pvStructure->getSubField<PVDouble>("value");
        if(!pvDouble) throw std::runtime_error("value is not a double");
        lock();
        try {
            beginGroupPut();
            pvValue->put(pvDouble->get());
            bool setAlarm = false;
            PVStructurePtr linkAlarmField = pvStructure->getSubField<PVStructure>("alarm");
            if(linkAlarmField && linkPVAlarm.attach(linkAlarmField)) {
                linkPVAlarm.get(linkAlarm);
            } else {
                linkAlarm.setMessage("connected");
                linkAlarm.setSeverity(noAlarm);
                linkAlarm.setStatus(clientStatus);
            }
            if(alarm.getMessage()!=linkAlarm.getMessage()) {
                alarm.setMessage(linkAlarm.getMessage());
                setAlarm = true;
            }
            if(alarm.getSeverity()!=linkAlarm.getSeverity()) {
                alarm.setSeverity(linkAlarm.getSeverity());
                setAlarm = true;
            }
            if(alarm.getStatus()!=linkAlarm.getStatus()) {
                alarm.setStatus(linkAlarm.getStatus());
                setAlarm = true;
             }
            if(setAlarm) pvAlarm.set(alarm);
            process();
            endGroupPut();
        } catch(...) {
           unlock();
           throw;
        }
        unlock();
        monitor->releaseEvent();
    }
}

}}
