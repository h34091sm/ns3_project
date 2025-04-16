#include "ns3/core-module.h"
#include "ns3/UrbanManetRoutingExperiment.h"

using namespace ns3;


int main(int argc, char *argv[]) { 
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::string experiment = argv[1];


    // Ptr<UrbanManetRoutingExperiment> manet_experiment;
    UrbanManetRoutingExperiment manet_experiment;

    if (experiment == "experiment1") 
    {
        for (int topologySize=20; topologySize<100; topologySize = topologySize + 10)
        { 
            manet_experiment.RunExperiment(experiment, topologySize, 2*topologySize, "AODV", 7.5, "64", "2048bps", "DsssRate11Mbps");
            manet_experiment.RunExperiment(experiment, topologySize, 2*topologySize, "DSDV", 7.5, "64", "2048bps", "DsssRate11Mbps");
        }
    }   
    
    else if (experiment == "experiment2") 
    {
        for (int gridSize=20; gridSize<200; gridSize = gridSize + 20)
        { 
            manet_experiment.RunExperiment(experiment, 60, gridSize, "AODV", 7.5, "64", "2048bps", "DsssRate11Mbps");
            manet_experiment.RunExperiment(experiment, 60, gridSize, "DSDV", 7.5, "64", "2048bps", "DsssRate11Mbps");
        }
    }


    return 0;
}