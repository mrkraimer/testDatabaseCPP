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
            if(printValue)  {
                 std::cout << "channelName " << channelName 
                           << " value " << monitorData->getValue()
                           << std::endl;
            }
            monitor->releaseEvent();
        }
    }
    PvaClientMonitorPtr getPvaClientMonitor() {
        return pvaClientMonitor;
    }

    void connectMonitor()
    {
        if(!channelConnected) {
            std::cout << "channelName " << channelName << " not connected\n";
            return;
        }
        if(!pvaClientMonitor) {
            pvaClientMonitor = pvaClientChannel->createMonitor(request);
            pvaClientMonitor->setRequester(shared_from_this());
            pvaClientMonitor->issueConnect();
        }
    }

    void stop()
    {
         if(!channelConnected || !monitorConnected)
         {
              std::cout << "channelName " << channelName << " not connected\n";
              return;
         }
         if(isStarted) {
             isStarted = false;
             pvaClientMonitor->stop();
         }
    }

    void start(const string &request)
    {
         if(!channelConnected || !monitorConnected)
         {
              std::cout << "channelName " << channelName << " not connected\n";
              return;
         }
         isStarted = true;
         pvaClientMonitor->start(request);
    }

};

typedef std::tr1::shared_ptr<ClientMonitor> ClientMonitorPtr;


int main(int argc,char *argv[])
{
    string provider("pva");
    epicsInt32 nchannels = 50000;
    epicsInt32 offset = 1;
    string request("value,alarm,timeStamp");
    double sleepTime = 0.0;
    string optString;
    bool debug(false);
    bool printValue(false);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:v:s:n:o:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << "-p provider -r request - d debug -v printValue -s sleepTime -n nchannels -o offset " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false")
                  << " -v " << (printValue ? "true" : "false")
                  << " -s " << sleepTime
                  << " -n " <<  nchannels
                  << " -o " <<  offset
                  << endl;           
                return 0;
            case 'd' :
               optString =  optarg;
               if(optString=="true") debug = true;
               break;
           case 'v' :
               optString =  optarg;
               if(optString=="true") printValue = true;
               break;
            case 's': 
                epicsScanDouble(optarg, &sleepTime);
                break;
            case 'n': 
                epicsParseInt32(optarg, &nchannels,10,NULL);
                break;
            case 'o': 
                epicsParseInt32(optarg, &offset,10,NULL);
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
         << " offset " <<  offset
         << " request " << request
         << " debug " << (debug ? "true" : "false")
         << " printValue " << (printValue ? "true" : "false")
         << endl;

    cout << "_____monitor starting__\n";
    
    try {   
        if(debug) PvaClient::setDebug(true);
        vector<string> channelNames;
        vector<ClientMonitorPtr> ClientMonitors;
        for(int i=offset; i< nchannels + offset; ++i) {
             std::ostringstream s;
             s <<"X";
             s << i;
             string channelName(s.str());
             channelNames.push_back(channelName);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        for(int i=0; i<nchannels; ++i) {
            ClientMonitors.push_back(
               ClientMonitor::create(
                   pva,channelNames[i],provider,request,printValue,sleepTime));
if(sleepTime>0.0) epicsThreadSleep(sleepTime);
        }
        while(true) {
            string str;
            getline(cin,str);
            if(str.compare("help")==0){
                 cout << "Type help exit status connectMonitor start stop\n";
                 continue;
            }
            if(str.compare("exit")==0) break;
            if(str.compare("connectMonitor")==0) {
                 for(int i=0; i<nchannels; ++i) {
                    try {
                       ClientMonitors[i]->connectMonitor();
                    } catch (std::exception& e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            if(str.compare("start")==0){
                 cout << "request?\n";
                 getline(cin,request);
                 for(int i=0; i<nchannels; ++i) {
                    try {
                       ClientMonitors[i]->start(request);
                    } catch (std::exception& e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            if(str.compare("stop")==0){
                 for(int i=0; i<nchannels; ++i) {
                    try {
                       ClientMonitors[i]->stop();
                    } catch (std::exception& e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            cout << str << " not a legal commnd\n";
        }
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
