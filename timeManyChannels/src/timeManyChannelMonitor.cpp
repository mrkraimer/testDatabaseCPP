/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <sstream>
#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsGuard.h>
#include <pv/pvaClient.h>
#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/convert.h>


using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;


typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientMonitor;
typedef std::tr1::shared_ptr<ClientMonitor> ClientMonitorPtr;

class ClientMonitor :
    public PvaClientChannelStateChangeRequester,
    public PvaClientMonitorRequester,
    public std::tr1::enable_shared_from_this<ClientMonitor>
{
private:
    string channelName;
    string provider;
    string request;
public:
    epicsUInt64 nreceived;
    epicsUInt64 nmissed;
    double oldvalue;
private:
    bool channelConnected;
    bool monitorConnected;
    bool isStarted;
    epics::pvData::Mutex mutex;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientMonitorPtr pvaClientMonitor;
    Event waitForCallback;  

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,provider);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
    }
public:
    POINTER_DEFINITIONS(ClientMonitor);
    ClientMonitor(
        const string &channelName,
        const string &provider,
        const string &request)
    : channelName(channelName),
      provider(provider),
      request(request),
      nreceived(0),
      nmissed(0),
      oldvalue(0.0),
      channelConnected(false),
      monitorConnected(false),
      isStarted(false)
    {
    }
    ~ClientMonitor()
     {
         //cout<< "~ClientMonitor() "<< channelName << "\n";
     }
    
    static ClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & provider,
        const string  & request)
    {
       ClientMonitorPtr client(ClientMonitorPtr(
             new ClientMonitor(channelName,provider,request)));
        client->init(pvaClient);
        client->issueConnect();
        return client;
    }

    void issueConnect()
    {
        pvaClientChannel->issueConnect();
    }

    bool isConnected()
    {
        {
           Lock xx(mutex);
           return (channelConnected ? true : false);
        }
    } 

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        if(isConnected) {
            if(!pvaClientMonitor) {
                pvaClientMonitor = pvaClientChannel->createMonitor(request);
                pvaClientMonitor->setRequester(shared_from_this());
                pvaClientMonitor->issueConnect();
            }
       }
       {
           Lock xx(mutex);
           if(isConnected) channelConnected = isConnected;
       }
    }

    virtual void monitorConnect(epics::pvData::Status const & status,
        PvaClientMonitorPtr const & monitor, epics::pvData::StructureConstPtr const & structure)
    {
        if(!status.isOK()) return;
        monitorConnected = true;
        if(isStarted) return;
        isStarted = true;
        pvaClientMonitor->start();
    }
    
    virtual void event(PvaClientMonitorPtr const & monitor)
    {
        while(monitor->poll()) {
            double value = monitor->getData()->getDouble();
            nreceived += 1;
            if(oldvalue>0.0) {
                long diff = value - oldvalue;
                nmissed = diff -1;
            }
            oldvalue = value;
            monitor->releaseEvent();
        }
    }

     void stop()
    {
         if(isStarted) {
             isStarted = false;
             pvaClientMonitor->stop();
         }
    }

    void start()
    {
         if(!channelConnected || !monitorConnected)
         {
              cout << "notconnected\n";
         }
         isStarted = true;
         pvaClientMonitor->start();
    }
};


int main(int argc,char *argv[])
{
    string argString("");
    string provider("pva");
    epicsInt32 nchannels = 50000;
    epicsInt32 offset = 1;
    string request("value,alarm,timeStamp");
    string optString;
    int opt;
    while((opt = getopt(argc, argv, "hp:n:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'h':
             cout << "-p provider -n nchannels  " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -n " <<  nchannels
                  << endl;           
                return 0;
            case 'n': 
                epicsParseInt32(optarg, &nchannels,10,NULL);
                break;
            case 'r':
                request = optarg;
                break;
            default:
                std::cerr<<"Unknown argument: "<<opt<<"\n";
                return -1;
        }
    }
    bool pvaSrv(((provider.find("pva")==string::npos) ? false : true));
    bool caSrv(((provider.find("ca")==string::npos) ? false : true));
    if(pvaSrv&&caSrv) {
        cerr<< "multiple providers are not allowed\n";
        return 1;
    }
    if(offset<0) offset=0;
    cout << "provider " << provider
         << " nchannels " <<  nchannels
         << endl;

    cout << "_____timeMultiChannel starting__\n";
    
    try {   
        vector<string> channelNames;
        vector<ClientMonitorPtr> ClientMonitor;
        for(int i=offset; i< nchannels + offset; ++i) {
             std::ostringstream s;
             s <<"X";
             s << i;
             string channelName(s.str());
             channelNames.push_back(channelName);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        TimeStamp startConnect;
        TimeStamp endConnect;
        startConnect.getCurrent();
        for(int i=0; i<nchannels; ++i) {
            ClientMonitor.push_back(
               ClientMonitor::create(
                   pva,channelNames[i],provider,request));
        }
        int numNotConnected = 0;
        while(true) {
            int numConnect = 0;
            for(int i=0; i<nchannels; ++i) {
                if(ClientMonitor[i]->isConnected()) numConnect++;
            }
            if(numConnect==nchannels) break;
            cout << "numConnect " << numConnect << "\n";
            epicsThreadSleep(1.0);
        }
        endConnect.getCurrent();
        cout << "enter something\n";
        string str;
        getline(cin,str);
        cout << "nchannels " << nchannels << " provider " << provider << "\n";
        cout << "numNotConnected " << numNotConnected << "\n";
        cout << "connect time " << TimeStamp::diff(endConnect,startConnect) << "\n";

        int numConnect = 0;
        long nreceived = 0;
        long nmissed = 0;
        for(int i=0; i<nchannels; ++i) {
              nreceived += ClientMonitor[i]->nreceived;
              nmissed += ClientMonitor[i]->nmissed;
        }
        cout << "TOTAL RECEIVED " << nreceived << "\n";
        cout << "TOTAL MISSED " << nmissed << "\n";
        for(int i=0; i<nchannels; ++i) {
            if(ClientMonitor[i]->isConnected()) numConnect++;
        }
        cout << " numConnect " << numConnect << "\n";
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
