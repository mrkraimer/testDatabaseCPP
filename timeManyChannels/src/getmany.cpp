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
cout << "channelStateChange "
<< channelName
<< " isConnected " << (isConnected? "true" : "false")
<< "\n";
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientGet) {
                pvaClientGet = pvaClientChannel->createGet(request);
                pvaClientGet->setRequester(shared_from_this());
                pvaClientGet->issueConnect();
            }
        } else {
            cout << channelName << " not connected\n";
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
        if(!status.isOK()) {
            cout << "channelGetDone " << channelName << " status " << status << endl;
        } else {
            cout << channelName << " " 
                 << clientGet->getData()->getPVStructure()->getSubField("value")
                 << "\n";
        }
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
        pvaClientGet->issueGet();
    }
   
};


int main(int argc,char *argv[])
{
    string provider("pva");
    epicsInt32 nchannels = 50000;
    epicsInt32 offset = 1;
    string request("value,alarm,timeStamp");
    string optString;
    bool debug(false);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:n:o:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << "-p provider -r request - d debug -n nchannels -o offset " << endl;
             cout << "default" << endl;
             cout << "-p " << provider 
                  << " -r " << request
                  << " -d " << (debug ? "true" : "false")
                  << " -n " <<  nchannels
                  << " -o " <<  offset
                  << endl;           
                return 0;
            case 'd' :
               optString =  optarg;
               if(optString=="true") debug = true;
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
         << endl;

    cout << "_____monitor starting__\n";
    
    try {   
        if(debug) PvaClient::setDebug(true);
        vector<string> channelNames;
        vector<ClientGetPtr> ClientGets;
        for(int i=offset; i< nchannels + offset; ++i) {
             std::ostringstream s;
             s <<"X";
             s << i;
             string channelName(s.str());
             channelNames.push_back(channelName);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        for(int i=0; i<nchannels; ++i) {
            ClientGets.push_back(
               ClientGet::create(
                   pva,channelNames[i],provider,request));
        }
        while(true) {
            cout << "Type help exit get\n";
            string str;
            getline(cin,str);
            if(str.compare("help")==0){
                 cout << "Type help exit get\n";
                 continue;
            }
            if(str.compare("exit")==0) break;
            if(str.compare("get")==0) {
                for(int i=0; i<nchannels; ++i) {
                    try {
                       ClientGets[i]->get();
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
