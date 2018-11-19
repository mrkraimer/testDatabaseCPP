/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <set>
#include <queue>
#include <vector>
#include <string>
#include <exception>

#include <epicsEvent.h>
#include <epicsMutex.h>
#include <epicsGuard.h>
#include <epicsGetopt.h>

#include <pv/configuration.h>
#include <pv/caProvider.h>
#include <pv/reftrack.h>
#include <pv/thread.h>
#include <pva/client.h>
#include <pv/timeStamp.h>

using namespace std;
using namespace epics::pvData;
using namespace epics::pvAccess;

namespace pvd = epics::pvData;
namespace pva = epics::pvAccess;

namespace {

typedef epicsGuard<epicsMutex> Guard;
typedef epicsGuardRelease<epicsMutex> UnGuard;

struct MonTracker : public pvac::ClientChannel::MonitorCallback,
                    public std::tr1::enable_shared_from_this<MonTracker>
{
    POINTER_DEFINITIONS(MonTracker);

    MonTracker(const std::string& name)
   :name(name),
    numMonitor(0)
    {
         timeStampLast.getCurrent();
    }
    virtual ~MonTracker() {
          mon.cancel();
    }

    const std::string name;
    pvac::Monitor mon;
    volatile long numMonitor;
    TimeStamp timeStamp;
    TimeStamp timeStampLast;
    epicsMutex mutex;

    virtual void monitorEvent(const pvac::MonitorEvent& evt) OVERRIDE FINAL
    {
        // shared_from_this() will fail as Cancel is delivered in our dtor.
        if(evt.event==pvac::MonitorEvent::Cancel) return;

        // running on internal provider worker thread
        // minimize work here.
        // TODO: bound queue size

        switch(evt.event) {
            case pvac::MonitorEvent::Fail:
                std::cout<<"Error "<<name<<" "<<evt.message<<"\n";
                break;
            case pvac::MonitorEvent::Cancel:
                std::cout<<"Cancel "<<name<<"\n";
                break;
            case pvac::MonitorEvent::Disconnect:
                std::cout<<"Disconnect "<<name<<"\n";
                break;
            case pvac::MonitorEvent::Data:
            {
                while(mon.poll())
                {
                    Guard G(mutex);
                    numMonitor++;
                }
            }
        }
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

} // namespace

int main(int argc, char *argv[]) {
    pva::ca::CAClientFactory::start();
    try {
        std::string providerName("pva"),
                    requestStr("value,alarm,timeStamp");
        typedef std::vector<std::string> pvs_t;
        pvs_t pvs;

        int opt;
        while((opt = getopt(argc, argv, "hp:w:r:")) != -1) {
            switch(opt) {
            case 'p':
                providerName = optarg;
                break;
            case 'r':
                requestStr = optarg;
                break;
            case 'h':
                std::cout<<"Usage: "<<argv[0]<<" [-p <provider>] [-w <timeout>] [-r <request>] [-R] <pvname> ...\n";
                return 0;
            default:
                std::cerr<<"Unknown argument: "<<opt<<"\n";
                return -1;
            }
        }

        for(int i=optind; i<argc; i++)
            pvs.push_back(argv[i]);

        // build "pvRequest" which asks for all fields
        pvd::PVStructure::shared_pointer pvReq(pvd::createRequest(requestStr));

        // explicitly select configuration from process environment
        pva::Configuration::shared_pointer conf(pva::ConfigurationBuilder()
                                                .push_env()
                                                .build());

        std::cout<<"Use provider: "<<providerName<<"\n";
        pvac::ClientProvider provider(providerName, conf);
        std::vector<MonTracker::shared_pointer> monitors;

        for(pvs_t::const_iterator it=pvs.begin(); it!=pvs.end(); ++it) {
            const std::string& pv = *it;
            MonTracker::shared_pointer mon(new MonTracker(pv));
            pvac::ClientChannel chan(provider.connect(pv));
            mon->mon = chan.monitor(mon.get(), pvReq);
            monitors.push_back(mon);
        }
        while(true) {
            string str;
            getline(cin,str);
            if(str.compare("exit")==0){
                 break;
            }
            if(str.compare("stop")==0){
                for(size_t i=0; i<monitors.size(); ++i){
                    monitors[i].reset();
                }
                continue;
            }
            if(str.compare("start")==0){
                for(size_t i=0; i<monitors.size(); ++i){
                    monitors[i] = MonTracker::shared_pointer(new MonTracker(pvs[i]));
                    pvac::ClientChannel chan(provider.connect(pvs[i]));
                    monitors[i]->mon = chan.monitor(monitors[i].get(), pvReq);
                }
                continue;
            }
            double events = 0.0;
            for(size_t i=0; i<monitors.size(); ++i) {
                MonTracker::shared_pointer mon(monitors[i]);
                if(mon) events += monitors[i]->report();
            }
            cout << "total events/second " << events << endl;
        }
        return 0;
    } catch(std::exception& e){
        std::cout<<"Error: "<<e.what()<<"\n";
        return 2;
    }
}
