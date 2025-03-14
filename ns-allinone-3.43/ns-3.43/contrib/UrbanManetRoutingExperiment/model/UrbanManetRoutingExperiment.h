#ifndef URBANMANETROUTINGEXPERIMENT_H
#define URBANMANETROUTINGEXPERIMENT_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/socket.h"
#include "ns3/aodv-module.h"

#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/olsr-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h" 
#include "ns3/yans-wifi-helper.h"

#include "ns3/CivillianNodes.h"
#include "ns3/EmergencyCentre.h"
#include "ns3/EmergencyResponder.h"
#include "ns3/UrbanManet.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <random>

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup UrbanManetRoutingExperiment Description of the UrbanManetRoutingExperiment
 */

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup UrbanManetRoutingExperiment directive

/* ... */
    class UrbanManetRoutingExperiment 
    {
        private: 
        std::vector<double> packetReceiveTimes;
        uint32_t bytesTotal;

        void WriteResultsToCsv(int topologySize, std::string routingProtocol, double pdr, double avgEndToEndDelay, double throughput);
        void MoveResponderToCaller(Ptr<Node> caller_node, Ptr<Node> responder_node, int &gridSize);
        Ptr<Node> NearestNodeToCaller(Ptr<Node> node, NodeContainer container);
        void FreezeNode(Ptr<Node> node, double interval);
        Ptr<Socket> RecvSocketConfig (Ptr<Node> node);
        void ReceivePacket(Ptr<Socket> socket) ;
        void SendPacket(Ptr<Socket> socket, Ipv4Address destination, uint16_t port, std::string msg) ;
        void EmergencyCallWithResponse(Ptr<Node> caller, NodeContainer &centre_container, NodeContainer &responder_container, UrbanManet &manet, int &gridSize);
        void ChangeNodeSize(AnimationInterface *anim, uint32_t nodeId, double newSize) ;



        public: 
        void RunExperiment(int topologySize, std::string protocolName, double txp, std::string packetSize, std::string rate, std::string phyMode) ;

    };
}


#endif /* URBANMANETROUTINGEXPERIMENT_H */
