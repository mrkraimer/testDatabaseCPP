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

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;
using namespace epics::pvaClient;

int main(int argc,char *argv[])
{
    string provider("pva");
    string channelName("TDBunion");
    string request("value");
    string debugString;
    bool debug(false);
    string scalarTypeName("");
    int opt;
    while((opt = getopt(argc, argv, "hr:c:s:d:")) != -1) {
        switch(opt) {
            case 'h':
             cout << " -h -r request -c channelName -s scalarTypeName - d debug value " << endl;
             cout << "default" << endl;
             cout << " -r " << request
                  << " -c " << channelName
                  << " -s -1 (use current type)"
                  << " -d " << (debug ? "true" : "false")
                  << " " <<  channelName
                  << endl;           
                return 0;
            case 'r':
                request = optarg;
                break;
            case 'c':
                channelName = optarg;
                break;
            case 's':
                scalarTypeName = optarg;
                break;
            case 'd' :
               debugString =  optarg;
               if(debugString=="true") debug = true;
               break;
            default:
                std::cerr<<"Unknown argument: "<<opt<<"\n";
                return -1;
        }
    }
    cout << "_____putUnion starting__\n";
    if(debug) PvaClient::setDebug(true);
    int nPvs = argc - optind;
    if(nPvs==0) {
        cerr<< "must provide a value\n";
        return 1;
    }
    string value(argv[optind]);
    size_t len = value.length();
    if(len<1) {
        cout << "value must have at least one character\n";
        return 1;
    }
    vector<string> values;
    bool isArray(true);
    size_t pos = 0;
    size_t offset = value.find('[',pos);
    if(offset==string::npos) {
        isArray = false;
    } else {
        size_t beg(offset+1);
        size_t end = value.find(']',offset);
        if(end==string::npos) {
            cout << "[ has no matching ]\n";
            return 1;
        }
        value = value.substr(beg,end-1);
        pos = 0;
        while(true)
        {
            size_t offset = value.find(',',pos);
            if(offset==string::npos) {
                values.push_back(value.substr(pos));
                break;
            }
            values.push_back(value.substr(pos,offset-pos));
            pos = offset+1;  
        }
    }
    PvaClientPtr pva= PvaClient::get(provider);
    try {
        PvaClientChannelPtr channel(pva->channel(channelName,provider,2.0));
        PvaClientPutPtr put(channel->put(request));
        PvaClientPutDataPtr putData = put->getData();
        PVFieldPtr pvField = putData->getPVStructure()->getSubField("value");
        if(!pvField) {
            throw std::runtime_error("no value field");
        }
        if(pvField->getField()->getType()!=union_) {
            throw std::runtime_error("value is not a PVUnion");
        }
        PVUnionPtr pvUnion = std::tr1::static_pointer_cast<PVUnion>(pvField);
        PVScalarPtr pvScalar;
        PVScalarArrayPtr pvScalarArray;
        if(scalarTypeName.length()>0) {
             if(isArray) {
                 pvScalarArray = getPVDataCreate()->
                     createPVScalarArray(ScalarTypeFunc::getScalarType(scalarTypeName));
             } else {
                 pvScalar = getPVDataCreate()->
                     createPVScalar(ScalarTypeFunc::getScalarType(scalarTypeName));
             }
        } else {
             PVFieldPtr pvField(pvUnion->get());
             if(!pvField) {
                 throw std::runtime_error("no value field");
             }
             if(isArray) {
                 if(pvField->getField()->getType()!=scalarArray) {
                     throw std::runtime_error("value is not a scalarrray");
                 }
                 pvScalarArray = std::tr1::static_pointer_cast<PVScalarArray>(pvField);
             } else {
                 if(pvField->getField()->getType()!=scalar) {
                     throw std::runtime_error("value is not a scalar");
                 }
                 pvScalar = std::tr1::static_pointer_cast<PVScalar>(pvField);
             }
       }
       if(isArray) {
           size_t len = values.size();
           pvScalarArray->setLength(len);
           getConvert()->fromStringArray(pvScalarArray,0,len,values,0);
           pvUnion->set(pvScalarArray);
       } else {
           getConvert()->fromString(pvScalar,value);
           pvUnion->set(pvScalar);
       }
       putData->getChangedBitSet()->set(pvUnion->getFieldOffset());
       put->put();
    } catch (std::exception& e) {
        cerr << "exception " << e.what() << endl;
        return 1;
    }
    return 0;
}
