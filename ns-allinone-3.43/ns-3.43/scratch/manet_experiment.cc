#include "ns3/core-module.h"
#include "ns3/UrbanManetRoutingExperiment.h"

using namespace ns3;


int main(int argc, char *argv[]) { 
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    int topologySize = std::stoi(argv[1]);
    std::string protocolName = argv[2];
    int number_of_runs = std::stoi(argv[3]);


    // Ptr<UrbanManetRoutingExperiment> manet_experiment;
    UrbanManetRoutingExperiment manet_experiment;


    if (topologySize < 10)
    {
        std::cout << "Need at least 10 nodes" << std::endl;
    }

    else
    {    

        if (protocolName != "OLSR" && protocolName != "DSDV" && protocolName != "DSR" && protocolName != "AODV")
        {
            std::cout << "Enter a valid protocol name: AODV, DSDV, OLSR or DSR" << std::endl;
        }
    
        else 
        {
            for (int i=0; i<number_of_runs; i++)
            { 
                manet_experiment.RunExperiment(topologySize, protocolName, 7.5, "64", "2048bps", "DsssRate11Mbps");
            }
    
        }
    }
    

    return 0;
}