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

class ClientChannel;
typedef std::tr1::shared_ptr<ClientChannel> ClientChannelPtr;

class ClientChannel :
    public PvaClientChannelStateChangeRequester,
    public std::tr1::enable_shared_from_this<ClientChannel>
{
private:
    string channelName;
    string provider;
    bool channelConnected;
    epics::pvData::Mutex mutex;

    PvaClientChannelPtr pvaClientChannel;
    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,provider);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientChannel);
    ClientChannel(
        const string &channelName,
        const string &provider)
    : channelName(channelName),
      provider(provider),
      channelConnected(false)
    {
    }
    ~ClientChannel() {
//         cout<< "~ClientChannel() "<< channelName << "\n";
    }
    
    static ClientChannelPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & provider)
    {
       ClientChannelPtr client(ClientChannelPtr(
             new ClientChannel(channelName,provider)));
        client->init(pvaClient);
        return client;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
       {
           Lock xx(mutex);
           if(isConnected) channelConnected = isConnected;
       }
    }

    bool isConnected()
    {
        {
           Lock xx(mutex);
           return channelConnected;
        }
    }

    void get()
    {
        if(!isConnected()) {
            cout << channelName << " channel not connected\n";
            return;
        }
        try {
            cout << "get " << channelName  << " " 
                 << pvaClientChannel->get()->getData()->getPVStructure()->getSubField("value") << endl;
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
    }

    void put(double value)
    {
        if(!channelConnected) {
            cout << channelName << " channel not connected\n";
            return;
        }
        try {
           PvaClientPutPtr put = pvaClientChannel->put();
           PvaClientPutDataPtr putData = put->getData();
           putData->putDouble(value); put->put();
        } catch (std::exception& e) {
            cerr << "exception " << e.what() << endl;
        }
    }
};



int main(int argc,char *argv[])
{
    string provider("pva");
    epicsInt32 nchannels = 50000;
    epicsInt32 offset = 1;
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
        vector<ClientChannelPtr> ClientChannel;
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
        TimeStamp startPut;
        TimeStamp endPut;
        TimeStamp startGet2;
        TimeStamp endGet2;
        startChannel.getCurrent();
        for(int i=0; i<nchannels; ++i) {
            ClientChannel.push_back(
               ClientChannel::create(
                   pva,channelNames[i],provider));
        }
        endChannel.getCurrent();
        int numNotConnected = 0;
        startWait.getCurrent();
        while(true) {
            int numConnect = 0;
            for(int i=0; i<nchannels; ++i) {
                if(ClientChannel[i]->isConnected()) numConnect++;
            }
            if(numConnect==nchannels) break;
            cout << "numConnect " << numConnect << "\n";
            epicsThreadSleep(1.0);
        }
        endWait.getCurrent();
        startGet1.getCurrent();
        for(int i=0; i<nchannels; ++i) {
             ClientChannel[i]->get();
        }
        endGet1.getCurrent();
        startPut.getCurrent();
        for(int i=0; i<nchannels; ++i) {
             ClientChannel[i]->put(1.0);      
        }
        endPut.getCurrent();
        startGet2.getCurrent();
        for(int i=0; i<nchannels; ++i) {
             ClientChannel[i]->get();      
        }
        endGet2.getCurrent();
        cout << "nchannels " << nchannels << " provider " << provider << "\n";
        cout << "numNotConnected " << numNotConnected << "\n";
        cout << "channel " << TimeStamp::diff(endChannel,startChannel) << "\n";
        cout << "wait " << TimeStamp::diff(endWait,startWait) << "\n";
        cout << "get1 " << TimeStamp::diff(endGet1,startGet1) << "\n";
        cout << "put " << TimeStamp::diff(endPut,startPut) << "\n";
        cout << "get2 " << TimeStamp::diff(endGet2,startGet2) << "\n";
        cout << "enter something\n";
        string str;
        getline(cin,str);
        int numConnect = 0;
        for(int i=0; i<nchannels; ++i) {
            if(ClientChannel[i]->isConnected()) numConnect++;
        }
        cout << " numConnect " << numConnect << "\n";
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
