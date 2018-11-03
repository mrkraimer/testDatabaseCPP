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
#include <pv/getLinkRecord.h>

using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;
using namespace epics::pvDatabase;
using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
using std::cout;
using std::endl;
using std::string;

namespace epics { namespace exampleCPP { namespace exampleLink {

class GetLinkRecordRequester :
    public PvaClientChannelStateChangeRequester,
    public PvaClientGetRequester
{
    GetLinkRecordWPtr exampleGetLinkRecord;
    PvaClient::weak_pointer pvaClient;
public:
    POINTER_DEFINITIONS(GetLinkRecordRequester);

    GetLinkRecordRequester(
        GetLinkRecordPtr const & exampleGetLinkRecord,
        PvaClientPtr const &pvaClient)
    : exampleGetLinkRecord(exampleGetLinkRecord),
      pvaClient(pvaClient)
    {}
    virtual ~GetLinkRecordRequester() {
        if(PvaClient::getDebug()) std::cout << "~GetLinkRecordRequester" << std::endl;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        GetLinkRecordPtr getLinkRecord(exampleGetLinkRecord.lock());
        if(!getLinkRecord) return;
        getLinkRecord->channelStateChange(channel,isConnected);  
    }

    virtual void channelGetConnect(
        const Status& status,
        PvaClientGetPtr const & clientGet)
    {
        GetLinkRecordPtr getLinkRecord(exampleGetLinkRecord.lock());
        if(!getLinkRecord) return;
        getLinkRecord->channelGetConnect(status,clientGet);  
    }

    virtual void getDone(
        const Status& status,
        PvaClientGetPtr const & clientGet) 
    {
// nothing to do
    }

};

GetLinkRecordPtr GetLinkRecord::create(
    PvaClientPtr  const & pva,
    string const & recordName,
    string const & providerName,
    string const & channelName)
{
    PVStructurePtr pvStructure = getStandardPVField()->scalar(pvDouble,"timeStamp,alarm");
    GetLinkRecordPtr pvRecord(
        new GetLinkRecord(
           recordName,pvStructure)); 
    GetLinkRecordRequesterPtr linkRecordRequester(
        GetLinkRecordRequesterPtr(new GetLinkRecordRequester(pvRecord,pva)));
    pvRecord->linkRecordRequester = linkRecordRequester;
    if(!pvRecord->init(pva,channelName,providerName)) pvRecord.reset();
    return pvRecord;
}

GetLinkRecord::GetLinkRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure),
  channelConnected(false),
  isGetConnected(false)
{
}


bool GetLinkRecord::init(
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

void GetLinkRecord::process()
{
    if(!channelConnected)
    {
        alarm.setMessage("disconnected");
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
    } else if(!isGetConnected) 
    {
        alarm.setMessage("channelGet not connected");
        alarm.setSeverity(invalidAlarm);
        pvAlarm.set(alarm);
    } else {
        try {
            pvaClientGet->get();
            double value = pvaClientGet->getData()->getDouble();
            pvValue->put(value);
            bool setAlarm = false;
            PVStructurePtr pvStructure = pvaClientGet->getData()->getPVStructure();
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
        } catch (std::runtime_error e) {
            alarm.setMessage(e.what());
            alarm.setSeverity(invalidAlarm);
            pvAlarm.set(alarm);
        }
    }
    PVRecord::process();
}

void GetLinkRecord::channelStateChange(
    PvaClientChannelPtr const & channel,
    bool isConnected)
{
    channelConnected = isConnected;
    if(isConnected) {
        if(!pvaClientGet) {
            pvaClientGet = pvaClientChannel->createGet("value,alarm");
            pvaClientGet->setRequester(linkRecordRequester);
            pvaClientGet->issueConnect();
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

void GetLinkRecord::channelGetConnect(
    const Status& status,
    PvaClientGetPtr const & clientGet)
{
    if(status.isOK()) {
        isGetConnected = true;
        return;
    }
    lock();
        isGetConnected = false;
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

}}}
