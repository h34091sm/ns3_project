#ifndef URBANMANET_H
#define URBANMANET_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/socket.h"
#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup UrbanManet Description of the UrbanManet
 */

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup UrbanManet directive

/* ... */
class UrbanManet
{
    private:
    std::string protocolName;
    double txp; 

    std::string rate; 
    std::string phyMode;
    Ipv4InterfaceContainer adhocInterfaces; 

    public:
    UrbanManet(std::string &protocolName, double &txp, std::string &packetSize, std::string &rate, std::string &phyMode, NodeContainer &nodes);
    ~UrbanManet(){}

    Ipv4InterfaceContainer get_adhocInterface()
    {
        return adhocInterfaces;
    }

};
}

#endif /* URBANMANET_H */
