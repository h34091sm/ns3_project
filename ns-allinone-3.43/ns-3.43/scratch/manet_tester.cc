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
#include "ns3/UrbanManet.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using namespace ns3;

std::vector<double> packetReceiveTimes;

void FreezeNode(Ptr<Node> node, Vector fixedPos, double interval)
{
  // Set the node's position to fixedPos and re-schedule this function
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
  mob->SetPosition(fixedPos);
  Simulator::Schedule(Seconds(interval), &FreezeNode, node, fixedPos, interval);
}
 
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
        NS_LOG_UNCOND("Time: " << (Simulator::Now()).GetSeconds() << " Received Packet: " 
                            << std::string((char*)buffer, packet->GetSize()) << std::endl);
    }
}


void SendPacket(Ptr<Socket> socket, Ipv4Address destination, uint16_t port, std::string msg) {
    Ptr<Packet> packet = Create<Packet>((uint8_t*)msg.c_str(), msg.size());
    socket->SendTo(packet, 0, InetSocketAddress(destination, port)); // Send packet to receiver
    NS_LOG_UNCOND("Time: " << (Simulator::Now()).GetSeconds() << " Sent Packet: " << msg);

}

void EmergencyCallWithResponse(Ptr<Node> centre, Ptr<Node> caller, double emergency_call_time, Ipv4Address centre_address, Ipv4Address caller_address)
{
    Ptr<Socket> recv_emergency_call = RecvSocketConfig(centre);
    recv_emergency_call->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_emergency_call = Socket::CreateSocket(caller, UdpSocketFactory::GetTypeId());  // Sender
    std::string message = "Emergency call by node: " + std::to_string(send_emergency_call->GetNode()->GetId())
    + ". At location " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().y);
    Simulator::Schedule(Seconds(emergency_call_time), &SendPacket, send_emergency_call, centre_address, 8080, message);

    Ptr<Socket> recv_centre_response = RecvSocketConfig(caller);
    recv_centre_response->SetRecvCallback(MakeCallback(&ReceivePacket));
    Ptr<Socket> send_centre_response = Socket::CreateSocket(centre, UdpSocketFactory::GetTypeId());  // Sender
    std::string message1 = "Emergency call by node: " + std::to_string(send_centre_response->GetNode()->GetId())
    + ". At location " + std::to_string(send_centre_response->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
    + ", " + std::to_string(send_centre_response->GetNode()->GetObject<MobilityModel>()->GetPosition().y);
    Simulator::Schedule(Seconds(emergency_call_time + 1.0), &SendPacket, send_centre_response, caller_address, 8080, message1);
}



int main(int argc, char *argv[]) { 
    // Enable logging
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Setup Emergency Centres 
    std::vector<ns3::Vector3D> centre_coords = {ns3::Vector3D(43.0, 20.0, 0.0), ns3::Vector3D(10.0, 20.0, 0.0), ns3::Vector3D(30.0, 20.0, 0.0)};
    ns3::EmergencyCentre emergency_centre;
    emergency_centre.add_nodes(centre_coords);
    NodeContainer centre_container = emergency_centre.get_container();

    // Setup Emergency Callers 
    std::vector<ns3::Vector3D> caller_coords = {ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), 
                                                ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), 
                                                ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0),
                                                ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0),
                                                ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0),
                                                ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0), ns3::Vector3D(rand() % 51, rand() % 51, 0.0)};

    ns3::EmergencyCaller emergency_caller;
    emergency_caller.add_nodes(caller_coords);
    NodeContainer caller_container = emergency_caller.get_container();

    // Node Container containing all nodes
    NodeContainer all_nodes;
    all_nodes.Add(centre_container, caller_container);

    //Configure MANET paramets
    std::string protocolName ("AODV");
    double txp(7.5);
    std::string packetSize("64");
    std::string rate("2048bps");
    std::string phyMode("DsssRate11Mbps");  

    UrbanManet manet = UrbanManet(protocolName, txp, packetSize, rate, phyMode, all_nodes);

    EmergencyCallWithResponse(centre_container.Get(0), caller_container.Get(1), 1.0, manet.get_adhocInterface().GetAddress(0), manet.get_adhocInterface().GetAddress(4));

    AnimationInterface anim ("manet-tester.xml");
    // Enable packet tracking lines
    anim.EnablePacketMetadata(true);

    // change colour and size of nodes

    for (auto j = centre_container.Begin(); j != centre_container.End(); ++j)
    {
        Ptr<Node> object = *j;
        anim.UpdateNodeColor (*j, 0, 255, 0); // Green
        anim.UpdateNodeSize (*j, 2, 2);
    }

 
    // Schedule Packet Sending

    Simulator::Schedule(Seconds(30.0), [&caller_container]() {
        Ptr<MobilityModel> mob = caller_container.Get(0)->GetObject<MobilityModel>();
        Vector freezePos = mob->GetPosition(); // Capture the position at stop time
        FreezeNode(caller_container.Get(0), freezePos, 1.0);
    });
    
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
