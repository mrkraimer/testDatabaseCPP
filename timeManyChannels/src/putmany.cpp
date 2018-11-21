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
#include <pv/convert.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

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
    
    static ClientPutPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & providerName,
        const string  & request)
    {
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
         cout << "putDone " << channelName << " status " << status << endl;
    }

    
    virtual void getDone(
        const epics::pvData::Status& status,
        PvaClientPutPtr const & clientPut)
    {
         cout << "getDone " << channelName << " status " << status << endl;
          if(status.isOK()) {
             cout << pvaClientPut->getData()->getPVStructure() << endl;
         } else {
             cout << "getGetDone " << channelName << " status " << status << endl;
         }
    }


    void put(const string & value)
    {
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


int main(int argc,char *argv[])
{
    string provider("pva");
    epicsInt32 nchannels = 50000;
    epicsInt32 offset = 1;
    string request("value,alarm,timeStamp");
    string optString;
    bool debug(false);
    int opt;
    while((opt = getopt(argc, argv, "hp:r:d:n:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'r':
                request = optarg;
                break;
            case 'h':
             cout << "-p provider -r request - d debug -n nchannels -o offset" << endl;
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
        vector<ClientPutPtr> ClientPuts;
        for(int i=offset; i< nchannels + offset; ++i) {
             std::ostringstream s;
             s <<"X";
             s << i;
             string channelName(s.str());
             channelNames.push_back(channelName);
        }
        PvaClientPtr pva= PvaClient::get(provider);
        for(int i=0; i<nchannels; ++i) {
            ClientPuts.push_back(
               ClientPut::create(
                   pva,channelNames[i],provider,request));
        }
        while(true) {
            cout << "enter one of: exit put get\n";
            int c = std::cin.peek();  // peek character
            if ( c == EOF ) continue;
            string str;
            getline(cin,str);
            if(str.compare("exit")==0) break;
            if(str.compare("get")==0) {
                 for(int i=0; i<nchannels; ++i) {
                    try {
                         ClientPuts[i]->get();
                    } catch (std::runtime_error e) {
                       cerr << "exception " << e.what() << endl;
                    }
                 }
                 continue;
            }
            if(str.compare("put")!=0) continue;
            cout << "enter value or values to put\n";
            getline(cin,str);
            for(int i=0; i<nchannels; ++i) {
                try {
                    ClientPuts[i]->put(str);
                } catch (std::runtime_error e) {
                   cerr << "exception " << e.what() << endl;
                }
            }
        }
    } catch (std::runtime_error e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
