/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <epicsGetopt.h>
#include <epicsGuard.h>
#include <pv/pvaClient.h>
#include <epicsThread.h>
#include <pv/timeStamp.h>


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
    string providerName;
    string request;
    bool channelConnected;
    bool monitorConnected;
    bool isStarted;
    long numMonitor;
    TimeStamp timeStamp;
    TimeStamp timeStampLast;
    epicsMutex mutex;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientMonitorPtr pvaClientMonitor;

    void init(PvaClientPtr const &pvaClient)
    {

        pvaClientChannel = pvaClient->createChannel(channelName,providerName);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }

public:
    POINTER_DEFINITIONS(ClientMonitor);
    ClientMonitor(
        const string &channelName,
        const string &providerName,
        const string &request)
    : channelName(channelName),
      providerName(providerName),
      request(request),
      channelConnected(false),
      monitorConnected(false),
      isStarted(false),
      numMonitor(0)
    {
        timeStampLast.getCurrent();
    }

    static ClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        const string  & request)
    {
        ClientMonitorPtr client(ClientMonitorPtr(
             new ClientMonitor(channelName,providerName,request)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientMonitor) {
                pvaClientMonitor = pvaClientChannel->createMonitor(request);
                pvaClientMonitor->setRequester(shared_from_this());
                pvaClientMonitor->issueConnect();
            }
        }
    }

    ClientMonitor()
    {
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
            {
                Guard G(mutex);
                numMonitor++;
            }
            monitor->releaseEvent();
        }
    }

    PvaClientMonitorPtr getPvaClientMonitor() {
        return pvaClientMonitor;
    }

    double report()
    {
        timeStamp.getCurrent();
        double  diff = TimeStamp::diff(timeStamp,timeStampLast);
        long numnow = 0;
        {
            Guard G(mutex);
            numnow = numMonitor;
            numMonitor = 0;
        }
        timeStampLast.getCurrent();
        double persecond = 0;
        if(diff>0.0) persecond = numnow/diff;
        return persecond;
    }

};

typedef std::tr1::shared_ptr<ClientMonitor> ClientMonitorPtr;


int main(int argc,char *argv[])
{
    string provider("pva");
    string channelName("FAST1");
    string request("value,alarm,timeStamp");
    string debugString;
    bool debug(false);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << " -h -p provider -r request - d debug channelNames " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false")
                  << " " <<  channelName
                  << endl;           
                return 0;
            case 'd' :
               debugString =  optarg;
               if(debugString=="true") debug = true;
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
    cout << "provider " << provider
         << " channelName " <<  channelName
         << " request " << request
         << " debug " << (debug ? "true" : "false") << endl;

    cout << "_____monitorrate starting__\n";
    PvaClientPtr pva(PvaClient::get(provider));
    try {   
        if(debug) PvaClient::setDebug(true);
        vector<string> channelNames;
        vector<ClientMonitorPtr> clientMonitors;
        int nPvs = argc - optind;       /* Remaining arg list are PV names */
        if (nPvs==0)
        {
            channelNames.push_back(channelName);
            nPvs = 1;
        } else {
            for (int n = 0; optind < argc; n++, optind++) channelNames.push_back(argv[optind]);
        }
        for(int i=0; i<nPvs; ++i) {
            clientMonitors.push_back(
                ClientMonitor::create(pva,channelNames[i],provider,request));
        }
        while(true) {
            string str;
            getline(cin,str);
            if(str.compare("stop")==0){
                  for(int i=0; i<nPvs; ++i){
                      PvaClientMonitorPtr pvaClientMonitor(clientMonitors[i]->getPvaClientMonitor());
                      if(pvaClientMonitor) pvaClientMonitor->stop();
                  }
                  continue;
            }
            if(str.compare("start")==0){
                  for(int i=0; i<nPvs; ++i){
                      PvaClientMonitorPtr pvaClientMonitor(clientMonitors[i]->getPvaClientMonitor());
                      if(pvaClientMonitor) pvaClientMonitor->start();
                  }
                  continue;
            }
            if(str.compare("exit")==0) break;
            double events = 0.0;
            for(int i=0; i<nPvs; ++i) {
                events += clientMonitors[i]->report();
            }
            cout << "total events/second " << events << endl;
        }
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
