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
    string putrequest;
    
    bool channelConnected;
    bool getConnected;
    epics::pvData::Mutex mutex;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientGetPtr pvaClientGet;
    
    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,provider);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
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
    ~ClientGet()
     {
         //cout<< "~ClientGet() "<< channelName << "\n";
     }
    
    static ClientGetPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & provider,
        const string  & request)
    {
       ClientGetPtr client(ClientGetPtr(
             new ClientGet(channelName,provider,request)));
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
            if(!pvaClientGet) {
                pvaClientGet = pvaClientChannel->createGet(request);
                pvaClientGet->setRequester(shared_from_this());
                pvaClientGet->issueConnect();
            }
       }
       {
           Lock xx(mutex);
           if(isConnected) channelConnected = isConnected;
       }
    }

    virtual void channelGetConnect(
        const epics::pvData::Status& status,
        PvaClientGetPtr const & clientGet)
    {
         getConnected = true;
//         cout << "channelGetConnect " << channelName << " status " << status << endl;
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
#ifdef XXX
            PvaClientGetDataPtr data = pvaClientGet->getData();
            cout << "get " << channelName << "\n";
            BitSetPtr bitSet =  data->getChangedBitSet();
            if(bitSet->cardinality()>0) {
                cout << "changed " << channelName << endl;
                data->showChanged(cout);
                cout << "bitSet " << *bitSet << endl;
            }
#endif
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
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
        vector<ClientGetPtr> ClientGet;
        for(int i=offset; i< nchannels + offset; ++i) {
             std::ostringstream s;
             s <<"X";
             s << i;
             string channelName(s.str());
             channelNames.push_back(channelName);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        TimeStamp startChannel;
        TimeStamp endChannel;
        TimeStamp startWait;
        TimeStamp endWait;
        TimeStamp startGet1;
        TimeStamp endGet1;
        TimeStamp startGet2;
        TimeStamp endGet2;
        startChannel.getCurrent();
        for(int i=0; i<nchannels; ++i) {
            ClientGet.push_back(
               ClientGet::create(
                   pva,channelNames[i],provider,request));
        }
        endChannel.getCurrent();
        int numNotConnected = 0;
        startWait.getCurrent();
        while(true) {
            int numConnect = 0;
            for(int i=0; i<nchannels; ++i) {
                if(ClientGet[i]->isConnected()) numConnect++;
            }
            if(numConnect==nchannels) break;
            cout << "numConnect " << numConnect << "\n";
            epicsThreadSleep(1.0);
        }
        endWait.getCurrent();
        startGet1.getCurrent();
        for(int i=0; i<nchannels; ++i) {
             ClientGet[i]->get();  
        }
        endGet1.getCurrent();
        startGet2.getCurrent();
        for(int i=0; i<nchannels; ++i) {
             ClientGet[i]->get();      
        }
        endGet2.getCurrent();
        cout << "nchannels " << nchannels << " provider " << provider << "\n";
        cout << "numNotConnected " << numNotConnected << "\n";
        cout << "channel " << TimeStamp::diff(endChannel,startChannel) << "\n";
        cout << "wait " << TimeStamp::diff(endWait,startWait) << "\n";
        cout << "get1 " << TimeStamp::diff(endGet1,startGet1) << "\n";
        cout << "get2 " << TimeStamp::diff(endGet2,startGet2) << "\n";
        cout << "enter something\n";
        string str;
        getline(cin,str);
        int numConnect = 0;
        for(int i=0; i<nchannels; ++i) {
            if(ClientGet[i]->isConnected()) numConnect++;
        }
        cout << " numConnect " << numConnect << "\n";
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
