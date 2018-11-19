/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>
#include <pv/pvaClient.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

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
    bool printValue;
    double sleepTime;
    bool channelConnected;
    bool monitorConnected;
    bool isStarted;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientMonitorPtr pvaClientMonitor;
    Mutex mutex;

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
        const string &request,
        const bool printValue,
        const double sleepTime)
    : channelName(channelName),
      providerName(providerName),
      request(request),
      printValue(printValue),
      sleepTime(sleepTime),
      channelConnected(false),
      monitorConnected(false),
      isStarted(false)
    {
    }

    static ClientMonitorPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        const string  & request,
        const bool printValue,
        const double sleepTime)
    {
        ClientMonitorPtr client(ClientMonitorPtr(
             new ClientMonitor(channelName,providerName,request,printValue,sleepTime)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        cout << "channelStateChange " << channelName << " isConnected " << (isConnected ? "true" : "false") << endl;
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
        cout << "monitorConnect " << channelName << " status " << status << endl;
        if(!status.isOK()) return;
        monitorConnected = true;
        if(isStarted) return;
        isStarted = true;
        pvaClientMonitor->start();
    }
    
    virtual void event(PvaClientMonitorPtr const & monitor)
    {
        if(sleepTime>0.0) epicsThreadSleep(sleepTime);
        while(monitor->poll()) {
            PvaClientMonitorDataPtr monitorData = monitor->getData();
             std::cout<<"Event "<< channelName
                        <<" Changed:" << *monitorData->getChangedBitSet()
                       <<" overrun:"<< *monitorData->getOverrunBitSet();
             if(printValue)  std::cout << " value " << monitorData->getValue();
             std::cout << std::endl;
            monitor->releaseEvent();
        }
    }
    PvaClientMonitorPtr getPvaClientMonitor() {
        return pvaClientMonitor;
    }

    void stop()
    {
         if(isStarted) {
             isStarted = false;
             pvaClientMonitor->stop();
         }
    }

    void start(const string &request)
    {
         if(!channelConnected || !monitorConnected)
         {
              cout << "notconnected\n";
         }
         isStarted = true;
         pvaClientMonitor->start(request);
    }

};

typedef std::tr1::shared_ptr<ClientMonitor> ClientMonitorPtr;


int main(int argc,char *argv[])
{
    string provider("pva");
    string channelName("PVRdouble");
    string request("value,alarm,timeStamp");
    double sleepTime = 0.0;
    string optString;
    bool debug(false);
    bool printValue(true);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:v:s:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << " -h -p provider -r request - d debug -v printValue -s 0.0 channelNames " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false")
                  << " -v " << (printValue ? "true" : "false")
                  << " -v " << sleepTime
                  << " " <<  channelName
                  << endl;           
                return 0;
            case 'd' :
               optString =  optarg;
               if(optString=="true") debug = true;
               break;
           case 'v' :
               optString =  optarg;
               if(optString=="false") printValue = false;
               break;
            case 's': 
                epicsScanDouble(optarg, &sleepTime);
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
         << " debug " << (debug ? "true" : "false")
         << " printValue " << (printValue ? "true" : "false")
         << endl;

    cout << "_____monitor starting__\n";
    
    try {   
        if(debug) PvaClient::setDebug(true);
        vector<string> channelNames;
        vector<ClientMonitorPtr> ClientMonitors;
        int nPvs = argc - optind;       /* Remaining arg list are PV names */
        if (nPvs==0)
        {
            channelNames.push_back(channelName);
            nPvs = 1;
        } else {
            for (int n = 0; optind < argc; n++, optind++) channelNames.push_back(argv[optind]);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        for(int i=0; i<nPvs; ++i) {
            ClientMonitors.push_back(
               ClientMonitor::create(
                   pva,channelNames[i],provider,request,printValue,sleepTime));
        }
        while(true) {
            string str;
            getline(cin,str);
            if(str.compare("help")==0){
                 cout << "Type help exit status start stop\n";
                 continue;
            }
            if(str.compare("exit")==0) break;
            if(str.compare("start")==0){
                 cout << "request?\n";
                 getline(cin,request);
                 for(int i=0; i<nPvs; ++i) {
                    try {
                       ClientMonitors[i]->start(request);
                    } catch (std::runtime_error e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            if(str.compare("stop")==0){
                 for(int i=0; i<nPvs; ++i) {
                    try {
                       ClientMonitors[i]->stop();
                    } catch (std::runtime_error e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            cout << str << " not a legal commnd\n";
        }
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
