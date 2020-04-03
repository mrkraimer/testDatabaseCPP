/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */

/**
 * @author mrk
 * @date 2013.04.02
 */

#include <pv/standardField.h>
#include <pv/ntscalar.h>

#include <epicsExport.h>
#include <pv/softRecord.h>


using namespace epics::pvData;
using namespace epics::pvDatabase;
using namespace epics::nt;
using std::tr1::static_pointer_cast;
using std::string;

namespace epics { namespace testDatabase {


SoftRecordPtr SoftRecord::create(
    string const & recordName)
{
    NTScalarBuilderPtr ntScalarBuilder = NTScalar::createBuilder();
    PVStructurePtr pvStructure = ntScalarBuilder->
        value(pvDouble)->
        addAlarm()->
        addTimeStamp()->
        addControl()->
        addDisplay()->
        createPVStructure();
    SoftRecordPtr pvRecord(
        new SoftRecord(recordName,pvStructure));
    pvRecord->initPVRecord();
    return pvRecord;
}

SoftRecord::SoftRecord(
    string const & recordName,
    PVStructurePtr const & pvStructure)
: PVRecord(recordName,pvStructure)
{
    PVFieldPtr pvField;
}

void SoftRecord::process()
{
   PVRecord::process();
}

}}
