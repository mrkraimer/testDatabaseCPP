/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 */

/* Author: Marty Kraimer */

#include <iostream>
#include <fstream>
#include <sstream>
#include <epicsGetopt.h>
#include <epicsGuard.h>
#include <pv/pvaClient.h>
#include <pv/pvAccess.h>
#include <epicsThread.h>
#include <pv/event.h>
#include <pv/timeStamp.h>
#include <pv/convert.h>
#include <iostream>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

class ClientGetPut;
typedef std::tr1::shared_ptr<ClientGetPut> ClientGetPutPtr;

class ClientGetPut :
    public PvaClientChannelStateChangeRequester,
    public PvaClientGetRequester,
    public PvaClientPutRequester,
    public std::tr1::enable_shared_from_this<ClientGetPut>
{
private:
    string channelName;
    string provider;
    string request;
    
    bool channelConnected;
    bool getConnected;
    bool putConnected;
    bool isStarted;

    PvaClientChannelPtr pvaClientChannel;
    PvaClientGetPtr pvaClientGet;
    PvaClientPutPtr pvaClientPut;
    Event waitForCallback;  

    void init(PvaClientPtr const &pvaClient)
    {
        pvaClientChannel = pvaClient->createChannel(channelName,provider);
        pvaClientChannel->setStateChangeRequester(shared_from_this());
        pvaClientChannel->issueConnect();
    }
public:
    POINTER_DEFINITIONS(ClientGetPut);
    ClientGetPut(
        const string &channelName,
        const string &provider,
        const string &request)
    : channelName(channelName),
      provider(provider),
      request(request),
      channelConnected(false),
      getConnected(false),
      putConnected(false),
      isStarted(false)
    {
    }
    ~ClientGetPut()
     {
//          cout<< "~ClientGetPut() "<< channelName << "\n";
     }
    
    static ClientGetPutPtr create(
        PvaClientPtr const &pvaClient,
        const string & channelName,
        const string & provider,
        const string  & request)
    {
       ClientGetPutPtr client(ClientGetPutPtr(
             new ClientGetPut(channelName,provider,request)));
        client->init(pvaClient);
        return client;
    }

    bool isConnected()
    {
         return channelConnected;
    }

    bool isGetConnected()
    {
        return getConnected;
    }

    bool isPutConnected()
    {
        return putConnected;
    }

    virtual void channelStateChange(PvaClientChannelPtr const & channel, bool isConnected)
    {
//cout << "channelStateChange isConnected " << (isConnected ? "true" : "false") << "\n";
        channelConnected = isConnected;
        if(isConnected) {
            if(!pvaClientGet) {
                pvaClientGet = pvaClientChannel->createGet(request);
                pvaClientGet->setRequester(shared_from_this());
                pvaClientGet->issueConnect();
            }
            if(!pvaClientPut) {
                pvaClientPut = pvaClientChannel->createPut(request);
                pvaClientPut->setRequester(shared_from_this());
                pvaClientPut->issueConnect();
            }
        }
    }

    virtual void channelGetConnect(
        const epics::pvData::Status& status,
        PvaClientGetPtr const & clientGet)
    {
//cout << "channelGetConnect\n";
         getConnected = true;
    }

    virtual void getDone(
        const epics::pvData::Status& status,
        PvaClientGetPtr const & clientGet)
    {
    }

    PVStructurePtr get()
    {
        pvaClientGet->get();
        PvaClientGetDataPtr data = pvaClientGet->getData();
        return data->getPVStructure();
    }
    
    virtual void channelPutConnect(
        const epics::pvData::Status& status,
        PvaClientPutPtr const & clientPut)
    {
//cout << "channelPutConnect\n";
        putConnected = true;
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
    }


    void put(const string & value)
    {
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
//      cout << "put pvStructure\n" << pvStructure << "\n";
        pvaClientPut->issuePut();
        waitForCallback.wait(); 
    }
};

static void putGet(
    ofstream &outfile,
    const PvaClientPtr &pva,
    const string& provider,
    const string& channelName,
    const string& request,
    const string& value)
{
    outfile << channelName
         << " provider " << provider
         << " request " << request
         << " value " << value
         << "\n";
    try {
        ClientGetPutPtr clientGetPut(
            ClientGetPut::create(pva,channelName,provider,request));
        int nwait = 0;
        while(true) {
            if(clientGetPut->isConnected()
                 && clientGetPut->isGetConnected()
                 && clientGetPut->isPutConnected()) break;
            epicsThreadSleep(.1);
            nwait++;
            if(nwait>20) {
                outfile << "did not connect\n";
                return;
            }
        }
        clientGetPut->put(value);
        PVStructurePtr pvResult(clientGetPut->get());
        outfile << "pvResult\n" << pvResult << "\n";
    } catch (std::exception& e) {
         outfile << "exception " << e.what() << endl;
    }
}

int main(int argc,char *argv[])
{
    string argString("");
    string provider("pva");
    string fileName("temp.txt");
    string channelName;
    string request;
    int opt;
    while((opt = getopt(argc, argv, "hp:f:")) != -1) {
        switch(opt) {
            case 'p':
                provider = optarg;
                break;
            case 'f':
                fileName = optarg;
                break;
            case 'h':
             cout << " -h -p provider -r request - d debug channelNames \n";
             cout << "default" << endl;
             cout << "-p " << provider
                  << " -f " << fileName
                  << endl;           
                return 0;
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
         << endl;

    cout << "_____exampleDatabase starting__\n";
    ofstream outfile;
    outfile.open (fileName);
//PvaClient::setDebug(true);
    PvaClientPtr pva(PvaClient::get("pva ca")); 
    // following is required or ChannelProviderRegistry goes away after each putGet
    ClientGetPutPtr clientGetPut(ClientGetPut::create(pva,"DBRint8",provider,"value"));

    putGet(outfile,pva,provider,"DBRint8","value","127");
    putGet(outfile,pva,provider,"DBRuint8","value[dbtype=DBF_UCHAR]","128");
    putGet(outfile,pva,provider,"DBRint16","value","32767");
    putGet(outfile,pva,provider,"DBRuint16","value[dbtype=DBF_USHORT]","32768");
    putGet(outfile,pva,provider,"DBRint32","value","2147483647");
    putGet(outfile,pva,provider,"DBRuint32","value[dbtype=DBF_ULONG]","4294967295");
    putGet(outfile,pva,provider,"DBRint64","value[dbtype=DBF_INT64]","4294967296000000001");
    putGet(outfile,pva,provider,"DBRuint64","value[dbtype=DBF_UINT64]","4294967296000000001");

    putGet(outfile,pva,provider,"DBRint8Array","value","127 -128");
    putGet(outfile,pva,provider,"DBRuint8Array","value[dbtype=DBF_UCHAR]","0 128");
    putGet(outfile,pva,provider,"DBRint16Array","value","32767 -32768");
    putGet(outfile,pva,provider,"DBRuint16Array","value[dbtype=DBF_USHORT]","0 32768");
    putGet(outfile,pva,provider,"DBRint32Array","value","2147483647 -2147483648");
    putGet(outfile,pva,provider,"DBRuint32Array","value[dbtype=DBF_ULONG]","0 4294967295");
    putGet(outfile,pva,provider,"DBRint64Array","value[dbtype=DBF_INT64]","4294967296000000001 -4294967296000000001");
    putGet(outfile,pva,provider,"DBRuint64Array","value[dbtype=DBF_UINT64]","0 4294967296000000001");
    putGet(outfile,pva,provider,"DBRstringArray","value","a b c d e f");

    putGet(outfile,pva,provider,"DBRlongstring.VAL$","value[pvtype=pvString]",
      "1234567892012345678930123456789401234567895012345678960");
    outfile.close();
    return 0;
}
