#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/socket.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h" 
#include "ns3/network-module.h"
#include "ns3/olsr-module.h"
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

using namespace ns3;

std::vector<double> packetReceiveTimes;


// Add this function to write results to a CSV file
void WriteResultsToCsv(int topologySize, std::string routingProtocol, double pdr, double avgEndToEndDelay) {

    std::string csv_filepath = "scratch/" + routingProtocol + "-manet-experiment-results.csv";
    std::ofstream outFile(csv_filepath, std::ios::app);  // Append mode
    if (!outFile) {
        NS_LOG_UNCOND("Error opening file for appending.");
        return;
    }

    // Write header only if the file is empty
    outFile.seekp(0, std::ios::end);
    if (outFile.tellp() == 0) {
        outFile << "Topology Size," << "Routing Protocol," << "Packet Delivery Ratio (%)," << "Average End-to-End Delay (s)," << "\n";
    }

    outFile << topologySize << ", " << routingProtocol << ", " << pdr << ", "<< avgEndToEndDelay << "\n";
    outFile.close();

    NS_LOG_UNCOND("Results appended to " << csv_filepath);
}



void MoveResponderToCaller(Ptr<Node> caller_node, Ptr<Node> responder_node, int &gridSize) 
{
    Ptr<ConstantVelocityMobilityModel> r_mobility = responder_node->GetObject<ConstantVelocityMobilityModel>();
    Ptr<RandomWaypointMobilityModel> c_mobility = caller_node->GetObject<RandomWaypointMobilityModel>();

    Vector velocity = Vector3D(c_mobility->GetPosition().x - r_mobility->GetPosition().x, c_mobility->GetPosition().y - r_mobility->GetPosition().y, 0.0);

    if (                                                            // if node position within range of +-1 of the destination coordinates
        (r_mobility->GetPosition().x >= c_mobility->GetPosition().x - 1) 
        && (r_mobility->GetPosition().x <= c_mobility->GetPosition().x  + 1) 
        && (r_mobility->GetPosition().y >= c_mobility->GetPosition().y - 1)
        && (r_mobility->GetPosition().y <= c_mobility->GetPosition().y + 1)
        )
    {
        r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));
        return;
    }

    else 
    {
        if (r_mobility->GetPosition().x > gridSize || r_mobility->GetPosition().x < 0 || r_mobility->GetPosition().y > gridSize || r_mobility->GetPosition().y < 0) // if node goes out of bounds
        {
            r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));
            return;
        }

        else 
        {
            r_mobility->SetVelocity(velocity);
            Simulator::Schedule(Seconds(1.0), &MoveResponderToCaller, caller_node, responder_node, gridSize);
        }
    }
}


// Determine Emergency Centre / Emergency Responder that is closest to caller
Ptr<Node> NearestNodeToCaller(Ptr<Node> node, NodeContainer container) {
    Ptr<MobilityModel> nodeMobility = node->GetObject<MobilityModel>();
    if (!nodeMobility) {
        NS_LOG_UNCOND("Node in container1 has no MobilityModel.");
        return nullptr;
    }

    Ptr<Node> nearestNode = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (uint32_t i = 0; i < container.GetN(); i++) 
    {
        Ptr<Node> currentNode = container.Get(i);
        Ptr<MobilityModel> currentMobility = currentNode->GetObject<MobilityModel>();
        if (!currentMobility) continue;

        double distance = nodeMobility->GetDistanceFrom(currentMobility);
        if (distance < minDistance) 
        {
            minDistance = distance;
            nearestNode = currentNode;
        }
    }

    return nearestNode;
}


// Stop a node that is experiencing emergency
void FreezeNode(Ptr<Node> node, double interval)
{
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
  mob->SetPosition(mob->GetPosition());

  Simulator::Schedule(Seconds(interval), &FreezeNode, node, interval);
}


// Receive packet functions 
Ptr<Socket> RecvSocketConfig (Ptr<Node> node)
{
    Ptr<Socket> recvSocket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 8080);
    recvSocket->Bind(local);  // Bind to port 8080
    return recvSocket;
}

void ReceivePacket(Ptr<Socket> socket) {
    Ptr<Packet> packet;
    while ((packet = socket->Recv())) {  // Receive the packet
        uint8_t buffer[1024];  // Buffer to store received data
        packet->CopyData(buffer, packet->GetSize());
        packetReceiveTimes.push_back((Simulator::Now()).GetSeconds());
        NS_LOG_UNCOND("Time = " << (Simulator::Now()).GetSeconds() << ": Packet received. " 
                            << std::string((char*)buffer, packet->GetSize()) << std::endl);
    }
}


// Send packet function 
void SendPacket(Ptr<Socket> socket, Ipv4Address destination, uint16_t port, std::string msg) {
    Ptr<Packet> packet = Create<Packet>((uint8_t*)msg.c_str(), msg.size());
    socket->SendTo(packet, 0, InetSocketAddress(destination, port)); // Send packet to receiver
    NS_LOG_UNCOND("Time = " << (Simulator::Now()).GetSeconds() << ": Packet Sent. " << msg);

}



void EmergencyCallWithResponse(Ptr<Node> caller, NodeContainer &centre_container, NodeContainer &responder_container, UrbanManet &manet, int &gridSize)
{

    Ptr<Node> nearestCentre = NearestNodeToCaller(caller, centre_container);
    Ipv4Address centre_address = manet.get_adhocInterface().GetAddress(nearestCentre->GetId());

    Ptr<Node> nearestResponder = NearestNodeToCaller(caller, responder_container);
    Ipv4Address responder_address = manet.get_adhocInterface().GetAddress(nearestResponder->GetId());

    std::cout << "\nCall by Caller Node: " << caller->GetId() << " Centre: " << nearestCentre->GetId() << " Responder: " << nearestResponder->GetId() << std::endl;


    Ptr<Socket> recv_emergency_call = RecvSocketConfig(nearestCentre);
    recv_emergency_call->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_emergency_call = Socket::CreateSocket(caller, UdpSocketFactory::GetTypeId());  // Sender

    std::string message = "Emergency call by node " + std::to_string(send_emergency_call->GetNode()->GetId()) 
    + " to node " +  std::to_string(recv_emergency_call->GetNode()->GetId()) 
    + ". At location " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().y);

    Simulator::Schedule(Seconds(0.0), &SendPacket, send_emergency_call, centre_address, 8080, message);


    Ptr<Socket> recv_centre_req = RecvSocketConfig(nearestResponder);
    recv_centre_req->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_centre_req = Socket::CreateSocket(nearestCentre, UdpSocketFactory::GetTypeId());  // Sender

    std::string message1 = "Centre Response by node " + std::to_string(send_centre_req->GetNode()->GetId())
    + " to node " +  std::to_string(recv_centre_req->GetNode()->GetId()) 
    + ". At location " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().y);

    Simulator::Schedule(Seconds(1.0), &SendPacket, send_centre_req, responder_address, 8080, message1); 

    Simulator::Schedule(Seconds(3.0), &MoveResponderToCaller, caller, nearestResponder, gridSize); 
} 


// To make a caller node disappear after responder has attended it 
void ChangeNodeSize(AnimationInterface *anim, uint32_t nodeId, double newSize) 
{
    anim->UpdateNodeSize(nodeId, newSize, newSize); // Width and height
}


void ManetExperiment(int topologySize, std::string protocolName, double txp, std::string packetSize, std::string rate, std::string phyMode) 
{
    int gridSize = 2 * topologySize;
    int numCivillians = topologySize * 0.7;
    int numCentres =  topologySize * 0.1; 
    int numResponders =  topologySize * 0.2;

    int numEmergencyCallers = topologySize * 0.3;

    int simulationTime = topologySize * 3;
    std::cout << "Simulation Time: " << simulationTime << std::endl;
    std::cout << "Grid Size: " << gridSize << std::endl;

    std::random_device rd;  // Obtain a random seed
    std::mt19937 gen(rd()); // Standard Mersenne Twister generator
    std::uniform_int_distribution<int> dist(0, gridSize); // Max Grid size is always between 0 and 4*numCivillians 

    // Configure Emergency Centre nodes 
    ns3::EmergencyCentre emergency_centre; 

    std::vector<ns3::Vector3D> centre_coords; 
    for (int i=0; i<numCentres; i++)
    {
        centre_coords.push_back(ns3::Vector3D(dist(gen), dist(gen), 0.0));
    }

    emergency_centre.add_nodes(centre_coords);
    NodeContainer centre_container = emergency_centre.get_container();

    // Configure Emergency Caller nodes 
    ns3::CivillianNodes civillian_nodes;

    std::vector<ns3::Vector3D> civillian_coords;
    for (int i=0; i<numCivillians; i++)
    {
        civillian_coords.push_back(ns3::Vector3D(dist(gen), dist(gen), 0.0));
    }

    civillian_nodes.add_nodes(civillian_coords);
    NodeContainer civillian_container = civillian_nodes.get_container();

    // Configure Emergency Responder nodes  
    ns3::EmergencyResponder emergency_responder;

    std::vector<ns3::Vector3D> responder_coords;
    for (int i=0; i<numResponders; i++)
    {
        responder_coords.push_back(ns3::Vector3D(dist(gen), dist(gen), 0.0));
    }
    emergency_responder.add_nodes(responder_coords);
    NodeContainer responder_container = emergency_responder.get_container();
     
    NodeContainer emergency_callers; // out of all the citizens, these are the ones that experience emergency

    for (int i=0; i<numEmergencyCallers; i++)
    {
        int rand_int = rand() % numCivillians;
        emergency_callers.Add(civillian_container.Get(rand_int));
    }

    // Node Container containing all nodes
    NodeContainer all_nodes;
    all_nodes.Add(centre_container, civillian_container, responder_container);

    UrbanManet manet = UrbanManet(protocolName, txp, packetSize, rate, phyMode, all_nodes);

    std::string animation_path_name = "scratch/" + protocolName + "-manet-experiment-" + std::to_string(topologySize) + ".xml";
    AnimationInterface anim (animation_path_name);
    anim.EnablePacketMetadata(true);

    // change colour and size of Emergency Centre nodes to distinguish it from other nodes
    for (auto j = centre_container.Begin(); j != centre_container.End(); ++j)
    {
        Ptr<Node> object = *j;
        anim.UpdateNodeColor (*j, 0, 255, 0); // Green
        anim.UpdateNodeSize (*j, 5, 5);
    }

    for (auto j = responder_container.Begin(); j != responder_container.End(); ++j)
    {
        Ptr<Node> object = *j;
        anim.UpdateNodeColor (*j, 0, 0, 255); // Blue
        anim.UpdateNodeSize (*j, 2, 2);
    }
  
    double time_value = 2.0;

    for (int i=0; i<numEmergencyCallers; i++)  // emergency simulation for callers that experience emergency
    {
        uint32_t caller_index = emergency_callers.Get(i)->GetId() - numCentres;
        uint32_t responder_index = NearestNodeToCaller(civillian_container.Get(caller_index), responder_container)->GetId() - numCivillians - numCentres;

        Ptr<RandomWaypointMobilityModel> caller_mobility = civillian_container.Get(caller_index)->GetObject<RandomWaypointMobilityModel>();
        Ptr<ConstantVelocityMobilityModel> responder_mobility = responder_container.Get(responder_index)->GetObject<ConstantVelocityMobilityModel>();

        Simulator::Schedule(Seconds(time_value), &FreezeNode, civillian_container.Get(caller_index), 1.0);

        anim.UpdateNodeColor (civillian_container.Get(caller_index), 2, 2, 2); // Black indicates that the nodes experience emergency

        Simulator::Schedule(Seconds(time_value + 1.0), &EmergencyCallWithResponse, civillian_container.Get(caller_index), centre_container, responder_container, manet, gridSize); 

        // Remove emergency caller that has been attended to
        Simulator::Schedule(Seconds(time_value + 6.0), &ChangeNodeSize, &anim, civillian_container.Get(caller_index)->GetId(), 0); 

        time_value = time_value + 10;
    }


    Simulator::Stop(Seconds(simulationTime));

    // Run Simulation
    // Flow Monitor
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    Simulator::Run();
    Simulator::Destroy();

    // Calculate metrics
    flowMonitor->CheckForLostPackets();

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

    double totalDelay = 0.0;
    uint64_t totalTxPackets = 0, totalRxPackets = 0;

    for (auto& flow : stats) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flow.first);
        NS_LOG_UNCOND("Flow " << flow.first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")");
        
        NS_LOG_UNCOND("Tx Packets: " << flow.second.txPackets);
        NS_LOG_UNCOND("Rx Packets: " << flow.second.rxPackets);
        
        if (flow.second.rxPackets > 0) {
            double avgDelay = (flow.second.delaySum.GetSeconds() / flow.second.rxPackets);
            NS_LOG_UNCOND("Avg End-to-End Delay: " << avgDelay << " seconds");
            totalDelay += flow.second.delaySum.GetSeconds();
        }

        totalTxPackets += flow.second.txPackets;
        totalRxPackets += flow.second.rxPackets;
    }

    // Compute and log overall results
    double pdr = (totalRxPackets * 100.0) / totalTxPackets;
    double avgEndToEndDelay = totalDelay / totalRxPackets;

    NS_LOG_UNCOND("Overall Packet Delivery Ratio: " << pdr << "%");
    NS_LOG_UNCOND("Overall Avg End-to-End Delay: " << avgEndToEndDelay << " seconds");

    // Insert this function call before the end of ManetExperiment()
    WriteResultsToCsv(topologySize, protocolName, pdr, avgEndToEndDelay);

    flowMonitor->SerializeToXmlFile("flow-monitor-results.xml", true, true);
}


int main(int argc, char *argv[]) { 
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    int topologySize = std::stoi(argv[1]);
    std::string protocolName = argv[2];


    if (protocolName != "OLSR" && protocolName != "DSDV" && protocolName != "DSR" && protocolName != "AODV")
    {
        std::cout << "Enter a valid protocol name: AODV, DSDV, OLSR or DSR" << std::endl;
    }

    else 
    {
        for (size_t i=0; i<3; i++)
        { 
            ManetExperiment(topologySize, protocolName, 7.5, "64", "2048bps", "DsssRate11Mbps");
        }

    }

    return 0;
}