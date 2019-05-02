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

static double sleepTime = 0.0;
static bool showGetPut = false; 

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientGet;
typedef std::tr1::shared_ptr<ClientGet> ClientGetPtr;

class ClientGet :
    public PvaClientChannelStateChangeRequester,
    public PvaClientGetRequester,
    public std::tr1::enable_shared_from_this<ClientGet>
{
private:
    string channelName;
    string provider;
    string request;
    bool channelConnected;
    bool getConnected;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientGetPtr pvaClientGet;

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,provider);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientGet);
    ClientGet(
        const string &channelName,
        const string &provider,
        const string &request)
    : channelName(channelName),
      provider(provider),
      request(request),
      channelConnected(false),
      getConnected(false)
    {
    }
    ~ClientGet() {cout<< "~ClientGet() "<< channelName << "\n";}
    
    static ClientGetPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & provider,
        const string  & request)
    {
       ClientGetPtr client(ClientGetPtr(
             new ClientGet(channelName,provider,request)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientGet) {
                pvaClientGet = pvaClientChannel->createGet(request);
                pvaClientGet->setRequester(shared_from_this());
                pvaClientGet->issueConnect();
            }
        }
    }

    virtual void channelGetConnect(
        const epics::pvData::Status& status,
        PvaClientGetPtr const & clientGet)
    {
         getConnected = true;
         cout << "channelGetConnect " << channelName << " status " << status << endl;
    }

    virtual void getDone(
        const epics::pvData::Status& status,
        PvaClientGetPtr const & clientGet)
    {
    }

    void get()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        if(!getConnected) {
            cout << channelName << " channelGet not connected\n";
            return;
        }
        try {
            pvaClientGet->get();
            PvaClientGetDataPtr data = pvaClientGet->getData();
            if(showGetPut) {
                cout << "get " << channelName << " " << data->getPVStructure()->getSubField("value") << endl;
            }
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
    }
   
};

class ClientPut;
typedef std::tr1::shared_ptr<ClientPut> ClientPutPtr;

class ClientPut :
    public PvaClientChannelStateChangeRequester,
    public PvaClientPutRequester,
    public std::tr1::enable_shared_from_this<ClientPut>
{
private:
    string channelName;
    string providerName;
    string request;
    bool channelConnected;
    bool putConnected;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientPutPtr pvaClientPut;
    Event waitForCallback;

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,providerName);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientPut);
    ClientPut(
        const string &channelName,
        const string &providerName,
        const string &request)
    : channelName(channelName),
      providerName(providerName),
      request(request),
      channelConnected(false),
      putConnected(false)
    {
    }
    ~ClientPut() {cout<< "~ClientPut() "<< channelName << "\n";}
    static ClientPutPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        bool block)
    {
        string request("field(value)");
        if(block) request = "record[block=true]field(value)";
        ClientPutPtr client(ClientPutPtr(
             new ClientPut(channelName,providerName,request)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientPut) {
                pvaClientPut = pvaClientChannel->createPut(request);
                pvaClientPut->setRequester(shared_from_this());
                pvaClientPut->issueConnect();
            }
        }
    }

    virtual void channelPutConnect(
        const epics::pvData::Status& status,
        PvaClientPutPtr const & clientPut)
    {
        putConnected = true;
        cout << "channelPutConnect " << channelName << " status " << status << endl;
    }

    
    virtual void putDone(
        const epics::pvData::Status& status,
        PvaClientPutPtr const & clientPut)
    {
         waitForCallback.signal();
    }

    
    virtual void getDone(
        const epics::pvData::Status& status,
        PvaClientPutPtr const & clientPut)
    {
         cout << "getDone " << channelName << " status " << status << endl;
          if(status.isOK()) {
             cout << pvaClientPut->getData()->getPVStructure() << endl;
         } else {
             cout << "getDone " << channelName << " status " << status << endl;
         }
    }


    void put(const string & value)
    {
        if(showGetPut) {
            cout << "put " << channelName << " value " << value << endl;
        }
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        if(!putConnected) {
            cout << channelName << " channelPut not connected\n";
            return;
        }
        PvaClientPutDataPtr putData = pvaClientPut->getData();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVScalarPtr pvScalar(pvStructure->getSubField<PVScalar>("value"));
        PVScalarArrayPtr pvScalarArray(pvStructure->getSubField<PVScalarArray>("value"));
        while(true) {
            if(pvScalar) break;
            if(pvScalarArray) break;
            PVFieldPtr pvField(pvStructure->getPVFields()[0]);
            pvScalar = std::tr1::dynamic_pointer_cast<PVScalar>(pvField);
            if(pvScalar) break;
            pvScalarArray = std::tr1::dynamic_pointer_cast<PVScalarArray>(pvField);
            if(pvScalarArray) break;
            pvStructure = std::tr1::dynamic_pointer_cast<PVStructure>(pvField);
            if(!pvStructure) {
               cout << channelName << " did not find a pvScalar field\n";
               return;
            }
        }
        ConvertPtr convert = getConvert();
        if(pvScalar) {
            convert->fromString(pvScalar,value);
        } else {
            vector<string> values;
            size_t pos = 0;
            size_t n = 1;
            while(true)
            {
                size_t offset = value.find(" ",pos);
                if(offset==string::npos) {
                    values.push_back(value.substr(pos));
                    break;
                }
                values.push_back(value.substr(pos,offset-pos));
                pos = offset+1;
                n++;    
            }
            pvScalarArray->setLength(n);
            convert->fromStringArray(pvScalarArray,0,n,values,0);        
        }
        pvaClientPut->issuePut();
        waitForCallback.wait(); 
    }

    void get()
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        if(!putConnected) {
            cout << channelName << " channelPut not connected\n";
            return;
        }
        pvaClientPut->issueGet();
    }
};


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
    ~ClientMonitor() {cout<< "~ClientMonitor() "<< channelName << "\n";}
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

class PutThread;
typedef std::tr1::shared_ptr<PutThread> PutThreadPtr;

class PutThread :
     public epicsThreadRunable
{
public:
    static PutThreadPtr create(
        PvaClientPtr const &pvaClient,
        const string &provider,
        vector<string> & channelNames,
        bool block);
    ~PutThread();
    virtual void run();
    void start();
    void stop();
private:
    PutThread(
        PvaClientPtr const &pvaClient,
        const string &provider,
        vector<string> & channelNames,
        bool block);

    bool isStop;
    PvaClientPtr pva;
    string provider;
    vector<string> channelNames;
    bool block;
    std::tr1::shared_ptr<epicsThread> thread;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForStop;
};

PutThreadPtr PutThread::create(
    PvaClientPtr const &pvaClient,
    const string &provider,
    vector<string> & channelNames,
    bool block)
{
    PutThreadPtr putThread(PutThreadPtr(new PutThread(pvaClient,provider,channelNames,block)));
    putThread->start();
    return putThread;
}

PutThread::PutThread(
    PvaClientPtr const &pvaClient,
    const string &provider,
    vector<string> & channelNames,
    bool block)
: isStop(false),
  pva(pvaClient),
  provider(provider),
  channelNames(channelNames),
  block(block)
{
}

PutThread::~PutThread()
{
std::cout << "PutThread::~PutThread()\n";
}

void PutThread::start()
{
    thread =  std::tr1::shared_ptr<epicsThread>(new epicsThread(
        *this,
        "PutThread",
        epicsThreadGetStackSize(epicsThreadStackSmall),
        epicsThreadPriorityLow));
    thread->start();  
}

void PutThread::stop()
{
cout << "PutThread::stop()\n";
    {
        Lock xx(mutex);
        isStop = true;
    }
cout << "PutThread::stop() call wait\n";
    waitForStop.wait();
cout << "PutThread::stop() after call wait\n";
}


void PutThread::run()
{
    vector<ClientPutPtr> clientPuts;
    int nPvs = channelNames.size();
    for(int i=0; i<nPvs; ++i) {
        clientPuts.push_back(
             ClientPut::create(pva,channelNames[i],provider,block));
    }
    int value = 0.0;
    while(true)
    {
         {
            Lock xx(mutex);
            if(isStop) {
                waitForStop.signal();
                return;
            }
         }
         std::ostringstream s;
         s << value;
         std::string val(s.str());
         value = value + 1.0;
         if(value>=10) value = 0.0;
         for(int i=0; i<nPvs; ++i)
         {         
             clientPuts[i]->put(val); 
         }
         if(sleepTime>0.0) epicsThreadSleep(sleepTime);
    }
}

class GetThread;
typedef std::tr1::shared_ptr<GetThread> GetThreadPtr;

class GetThread :
     public epicsThreadRunable
{
public:
    static GetThreadPtr create(
        PvaClientPtr const &pvaClient,
        const string &provider,
        vector<string> & channelNames);
    ~GetThread();
    virtual void run();
    void start();
    void stop();
private:
    GetThread(
        PvaClientPtr const &pvaClient,
        const string &provider,
        vector<string> & channelNames);

    bool isStop;
    PvaClientPtr pva;
    string provider;
    vector<string> channelNames;
    std::tr1::shared_ptr<epicsThread> thread;
    epics::pvData::Mutex mutex;
    epics::pvData::Event waitForStop;
};

GetThreadPtr GetThread::create(
    PvaClientPtr const &pvaClient,
    const string &provider,
    vector<string> & channelNames)
{
    GetThreadPtr getThread(GetThreadPtr(new GetThread(pvaClient,provider,channelNames)));
    getThread->start();
    return getThread;
}

GetThread::GetThread(
    PvaClientPtr const &pvaClient,
    const string &provider,
    vector<string> & channelNames)
: isStop(false),
  pva(pvaClient),
  provider(provider),
  channelNames(channelNames)
{
}

GetThread::~GetThread()
{
std::cout << "GetThread::~GetThread()\n";
}

void GetThread::start()
{
    thread =  std::tr1::shared_ptr<epicsThread>(new epicsThread(
        *this,
        "GetThread",
        epicsThreadGetStackSize(epicsThreadStackSmall),
        epicsThreadPriorityLow));
    thread->start();  
}

void GetThread::stop()
{
    {
        Lock xx(mutex);
        isStop = true;
    }

    waitForStop.wait();
}


void GetThread::run()
{
    vector<ClientGetPtr> clientGets;
    int nPvs = channelNames.size();
    for(int i=0; i<nPvs; ++i) {
        clientGets.push_back(
            ClientGet::create(pva,channelNames[i],provider,"value"));
    }
    while(true)
    {
         {
            Lock xx(mutex);
            if(isStop) {
                waitForStop.signal();
                return;
            }
         }
         for(int i=0; i<nPvs; ++i)
         {
             clientGets[i]->get();
         }
         if(sleepTime>0.0) epicsThreadSleep(sleepTime);
    }
}



int main(int argc,char *argv[])
{
    string argString("");
    string provider("pva");
    string channelName("PVRdouble");
    string request("value,alarm,timeStamp");
    bool debug(false);
    bool block(true);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:s:o:b:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << " -h -p provider -r request - d debug -s sleepTime -o showGetPut channelNames \n";
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false")
                  << " -s " << sleepTime
                  << " -o " << (showGetPut ? "true" : "false")
                  << " -b " << (block ? "true" : "false")
                  << " " <<  channelName
                  << endl;           
                return 0;
            case 'd' :
               argString = optarg;
               if(argString=="true") debug = true;
               break;
            case 's' :
               argString = optarg;
               sleepTime =  std::stod(argString);
               break;
            case 'o' :
               argString = optarg;
               if(argString=="true") showGetPut = true;
               break;
            case 'b' :
               argString = optarg;
               if(argString=="true") block = true;
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

    cout << "_____getputmonitorrate starting__\n";
    PvaClientPtr pva(PvaClient::get(provider));
    try {  
        if(debug) PvaClient::setDebug(true);
        vector<string> channelNames;
        vector<ClientMonitorPtr> clientMonitors;
        vector<ClientPutPtr> clientPuts;
        vector<ClientGetPtr> clientGets;
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
        PutThreadPtr putThread;
        GetThreadPtr getThread;
        while(true) {
            cout << "enter one of: exit start stop putThread getThread getLocal putLocal get put\n";
            string str;
            getline(cin,str);
            if(str.compare("putThread")==0){
                 if(putThread) {
                     putThread->stop();
                     putThread.reset();
                 } else {
                     cout << "enter channelNames\n";
                     string value;
                     getline(cin,value);
                     vector<string> channelNames;
                     size_t pos = 0;
                     size_t n = 1;
                     while(true)
                     {
                         size_t offset = value.find(" ",pos);
                         if(offset==string::npos) {
                              channelNames.push_back(value.substr(pos));
                              break;
                         }
                         channelNames.push_back(value.substr(pos,offset-pos));
                         pos = offset+1;
                         n++;    
                     }
                     putThread = PutThread::create(pva,provider,channelNames,block);
                 }
                 continue;
            }
            if(str.compare("getThread")==0){
                 if(getThread) {
                     getThread->stop();
                     getThread.reset();
                 } else {
                     getThread = GetThread::create(pva,provider,channelNames);
                 }
                 continue;
            }
            if(str.compare("putLocal")==0){
                 if(clientPuts.size()==0) {
                      for(int i=0; i<nPvs; ++i) {
                           clientPuts.push_back(
                           ClientPut::create(pva,channelNames[i],provider,block));
                      }
                 } else {
                      clientPuts.erase(clientPuts.begin(),clientPuts.end());
                 }
                 continue;
            }
            if(str.compare("getLocal")==0){
                 if(clientGets.size()==0) {
                      for(int i=0; i<nPvs; ++i) {
                           clientGets.push_back(
                           ClientGet::create(pva,channelNames[i],provider,request));
                      }
                 } else {
                      clientGets.erase(clientGets.begin(),clientGets.end());
                 }
                 continue;
            }
            if(str.compare("get")==0){
                 bool save = showGetPut;
                 showGetPut = true;
                 for(size_t i=0; i< clientGets.size() ; ++i) clientGets[i]->get();
                 showGetPut = save;
                 continue;
            }
            if(str.compare("put")==0){
                 bool save = showGetPut;
                 showGetPut = true;
                 string val("0");
                 for(size_t i=0; i< clientPuts.size() ; ++i) clientPuts[i]->put(val);
                 showGetPut = save;
                 continue;
            }
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
            if(str.compare("exit")==0)
            {
                 if(putThread) putThread->stop();
                 if(getThread) getThread->stop();
                 break;
            }
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
