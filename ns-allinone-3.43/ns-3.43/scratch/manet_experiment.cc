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
#include "ns3/EmergencyCaller.h"
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

void RemoveNode(Ptr<Node> node) {
    // Step 1: Stop all applications on the node
    uint32_t appCount = node->GetNApplications();
    for (uint32_t i = 0; i < appCount; ++i) {
        node->GetApplication(i)->SetStopTime(Seconds(0));  // Correct way to stop apps
    }

    // Step 2: Dispose of all NetDevices on the node
    uint32_t deviceCount = node->GetNDevices();
    for (uint32_t i = 0; i < deviceCount; ++i) {
        node->GetDevice(i)->Dispose();  // Dispose of each NetDevice
    }

    // Step 3: Dispose of the node itself
    node->Dispose();
}


void MoveResponderToCaller(Ptr<Node> caller_node, Ptr<Node> responder_node) 
{
    Ptr<ConstantVelocityMobilityModel> r_mobility = responder_node->GetObject<ConstantVelocityMobilityModel>();
    Ptr<RandomWaypointMobilityModel> c_mobility = caller_node->GetObject<RandomWaypointMobilityModel>();

    Vector velocity = Vector3D(c_mobility->GetPosition().x - r_mobility->GetPosition().x, c_mobility->GetPosition().y - r_mobility->GetPosition().y, 0.0);

    if (                                                            // if node position within range of +-1 of the destination coordinates
        (r_mobility->GetPosition().x >= c_mobility->GetPosition().x - 1) 
        && (r_mobility->GetPosition().x <= c_mobility->GetPosition().x  + 1) 
        && (r_mobility->GetPosition().y >=c_mobility->GetPosition().y - 1)
        && (r_mobility->GetPosition().y <= c_mobility->GetPosition().y + 1)
        )
    {
        r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));
        return;
    }

    else 
    {
        if (r_mobility->GetPosition().x > 100 || r_mobility->GetPosition().x < 0 || r_mobility->GetPosition().y > 100 || r_mobility->GetPosition().y < 0) // if node goes out of bounds
        {
            r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));
            return;
        }

        else 
        {
            r_mobility->SetVelocity(velocity);
            Simulator::Schedule(Seconds(1.0), &MoveResponderToCaller, caller_node, responder_node);
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

    for (uint32_t i = 0; i < container.GetN(); i++) {
        Ptr<Node> currentNode = container.Get(i);
        Ptr<MobilityModel> currentMobility = currentNode->GetObject<MobilityModel>();
        if (!currentMobility) continue;

        double distance = nodeMobility->GetDistanceFrom(currentMobility);
        if (distance < minDistance) {
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

// Caller calls centre, centre informs responder and responder attends caller
void EmergencyCallWithResponse(Ptr<Node> centre, Ptr<Node> caller, Ptr<Node> responder,  double emergency_call_time, Ipv4Address centre_address, Ipv4Address caller_address, Ipv4Address responder_address)
{
    std::cout << "Call by Caller Node: " << caller->GetId() << " Centre: " << centre->GetId() << " Responder: " << responder->GetId() << std::endl;
    Ptr<Socket> recv_emergency_call = RecvSocketConfig(centre);
    recv_emergency_call->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_emergency_call = Socket::CreateSocket(caller, UdpSocketFactory::GetTypeId());  // Sender
    std::string message = "Emergency call by node " + std::to_string(send_emergency_call->GetNode()->GetId()) 
    + " to node " +  std::to_string(recv_emergency_call->GetNode()->GetId()) 
    + ". At location " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().y);
    Simulator::Schedule(Seconds(emergency_call_time), &SendPacket, send_emergency_call, centre_address, 8080, message);

    Ptr<Socket> recv_centre_req = RecvSocketConfig(responder);
    recv_centre_req->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_centre_req = Socket::CreateSocket(centre, UdpSocketFactory::GetTypeId());  // Sender
    std::string message1 = "Centre Response by node " + std::to_string(send_centre_req->GetNode()->GetId())
    + " to node " +  std::to_string(recv_centre_req->GetNode()->GetId()) 
    + ". At location " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().y);
    Simulator::Schedule(Seconds(emergency_call_time + 1.0), &SendPacket, send_centre_req, responder_address, 8080, message1); 

}

// To make a caller node disappear after responder has attended it 
void ChangeNodeSize(AnimationInterface *anim, uint32_t nodeId, double newSize) {
    anim->UpdateNodeSize(nodeId, newSize, newSize); // Width and height
}

int main(int argc, char *argv[]) { 
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    std::random_device rd;  // Obtain a random seed
    std::mt19937 gen(rd()); // Standard Mersenne Twister generator
    std::uniform_int_distribution<int> dist(0, 100); // Define range  

    // Configure Emergency Centre nodes 
    ns3::EmergencyCentre emergency_centre;
    std::vector<ns3::Vector3D> centre_coords = {
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0),
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0) 
                                               };

    emergency_centre.add_nodes(centre_coords);
    NodeContainer centre_container = emergency_centre.get_container();


    // Configure Emergency Caller nodes 
    ns3::EmergencyCaller emergency_caller;
    std::vector<ns3::Vector3D> caller_coords = {
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0),
                                                ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0),
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0),
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
                                                // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0)
                                               };

    emergency_caller.add_nodes(caller_coords);
    NodeContainer caller_container = emergency_caller.get_container();
    uint32_t numCallers = caller_container.GetN();

    // Configure Emergency Responder nodes 
    ns3::EmergencyResponder emergency_responder;
    std::vector<ns3::Vector3D> responder_coords = {
        ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
        // ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), ns3::Vector3D(dist(gen), dist(gen), 0.0), 
    };
    emergency_responder.add_nodes(responder_coords);
    NodeContainer responder_container = emergency_responder.get_container();
    // uint32_t numResponders = responder_container.GetN();
     

    NodeContainer callers_which_experience_emergency; // out of all the citizens, these are the ones that experience emergency

    for (uint32_t i=0; i<7; i++)
    {
        uint32_t rand_int = rand() % numCallers;
        callers_which_experience_emergency.Add(caller_container.Get(rand_int));
    }

    // Node Container containing all nodes
    NodeContainer all_nodes;
    all_nodes.Add(centre_container, caller_container, responder_container);
 
    // Configure MANET parameters
    std::string protocolName ("AODV");
    double txp(7.5);
    std::string packetSize("64");
    std::string rate("2048bps");
    std::string phyMode("DsssRate11Mbps");  

    UrbanManet manet = UrbanManet(protocolName, txp, packetSize, rate, phyMode, all_nodes);

    AnimationInterface anim ("manet-experiment.xml");

    // Enable packet tracking lines
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
  
    uint32_t numCentres = centre_container.GetN();
    double time_value = 2.0;

    for (uint32_t i=0; i<callers_which_experience_emergency.GetN(); i++)  // emergency simulation for callers that experience emergency
    {
        uint32_t caller_index = callers_which_experience_emergency.Get(i)->GetId() - numCentres;
        uint32_t centre_index = NearestNodeToCaller(caller_container.Get(caller_index), centre_container)->GetId();
        uint32_t responder_index = NearestNodeToCaller(caller_container.Get(caller_index), responder_container)->GetId() - numCallers - numCentres;

        Ptr<RandomWaypointMobilityModel> caller_mobility = caller_container.Get(caller_index)->GetObject<RandomWaypointMobilityModel>();
        Ptr<ConstantVelocityMobilityModel> responder_mobility = responder_container.Get(responder_index)->GetObject<ConstantVelocityMobilityModel>();

        Simulator::Schedule(Seconds(time_value), &FreezeNode, caller_container.Get(caller_index), 1.0);

        anim.UpdateNodeColor (caller_container.Get(caller_index), 2, 2, 2); // Black indicates that the nodes experience emergency
        EmergencyCallWithResponse(centre_container.Get(centre_index), caller_container.Get(caller_index), responder_container.Get(responder_index), time_value+1.0, 
                                  manet.get_adhocInterface().GetAddress(centre_index), manet.get_adhocInterface().GetAddress(callers_which_experience_emergency.Get(i)->GetId()), 
                                 manet.get_adhocInterface().GetAddress(responder_index + numCallers + numCentres));

        Simulator::Schedule(Seconds(time_value + 4.0), &MoveResponderToCaller, caller_container.Get(caller_index), responder_container.Get(responder_index)); 

        // Remove emergency caller that has been attended to
        Simulator::Schedule(Seconds(time_value + 5.0), &ChangeNodeSize, &anim, caller_container.Get(caller_index)->GetId(), 0); 

        time_value = time_value + 10;
}


    Simulator::Stop(Seconds(100));

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

    flowMonitor->SerializeToXmlFile("flow-monitor-results.xml", true, true);

    return 0;
}