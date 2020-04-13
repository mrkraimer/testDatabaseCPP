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
#include <pv/pvaClient.h>
#include <pv/convert.h>
#include <pv/standardField.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

int main(int argc,char *argv[])
{
    string provider("pva");
    string addRecordName("TDBaddRecord");
    string recordName("TDBunion");
    string traceRecordName("TDBtraceRecord");
    string debugString;
    bool debug(false);
    int opt;
    while((opt = getopt(argc, argv, "hd:a:")) != -1) {
        switch(opt) {
            case 'h':
             cout << " -h - d debug " << endl;
             cout << "default" << endl;
             cout << " -d " << (debug ? "true" : "false") << endl;           
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
    
    cout << "provider " << provider
         << " addRecordName " <<  addRecordName
         << " debug " << (debug ? "true" : "false")
         << endl;

    cout << "_____addUnionRecord starting__\n";
    
    try {   
        if(debug) PvaClient::setDebug(true);
        PvaClientPtr pvaClient= PvaClient::get(provider);
        PvaClientChannelPtr pvaClientChannel(pvaClient->createChannel(addRecordName,provider));
        pvaClientChannel->connect();
        StandardFieldPtr standardField = getStandardField();
        FieldCreatePtr fieldCreate = getFieldCreate();
        StructureConstPtr top = fieldCreate->createFieldBuilder()->
            add("value",fieldCreate->createVariantUnion()) ->
            add("timeStamp", standardField->timeStamp()) ->
            addNestedStructure("subfield") ->
               add("value",fieldCreate->createVariantUnion()) ->
               endNested()->
            createStructure();
        PVStructurePtr pvValue = getPVDataCreate()->createPVStructure(top);
        PvaClientPutGetPtr pvaClientPutGet(pvaClientChannel->createPutGet());
        PvaClientPutDataPtr putData = pvaClientPutGet->getPutData();
        PVStructurePtr pvStructure = putData->getPVStructure();
        PVStringPtr pvName = pvStructure->getSubField<PVString>("argument.recordName");
        if(!pvName) {
             cout << "argument.recordName not found\n";
        }
        pvName->put(recordName);
        PVUnionPtr pvUnion = pvStructure->getSubField<PVUnion>("argument.union");
        if(!pvUnion) {
             cout << "argument.union not found\n";
        }
        pvUnion->set(pvValue);
        putData->getChangedBitSet()->set(pvUnion->getFieldOffset());
        pvaClientPutGet->putGet();
        PvaClientGetDataPtr getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
        if(!debug) return 0;
        pvaClientChannel= pvaClient->createChannel(traceRecordName,provider);
        pvaClientChannel->connect();
        pvaClientPutGet = pvaClientChannel->createPutGet();
        putData = pvaClientPutGet->getPutData();
        pvStructure = putData->getPVStructure();
        pvName = pvStructure->getSubField<PVString>("argument.recordName");
        if(!pvName) {
             cout << "argument.recordName not found\n";
        }
        pvName->put(recordName);
        PVIntPtr pvLevel = pvStructure->getSubField<PVInt>("argument.level");
        if(!pvLevel) {
             cout << "argument.level not found\n";
        }
        pvLevel->put(3);
        pvaClientPutGet->putGet();
        getData = pvaClientPutGet->getGetData();
        cout << getData->getPVStructure() << endl;
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
