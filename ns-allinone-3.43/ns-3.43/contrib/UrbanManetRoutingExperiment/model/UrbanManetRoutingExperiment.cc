#include "UrbanManetRoutingExperiment.h"

namespace ns3
{

/* ... */
    // Add this function to write results to a CSV file
    void UrbanManetRoutingExperiment::WriteResultsToCsv(std::string experiment, int topology_or_grid_size, std::string routingProtocol, double pdr, double avgEndToEndDelay, double throughput) 
    {
        std::string csv_filepath = "scratch/" + experiment + "/manet-experiment-results.csv";
        std::ofstream outFile(csv_filepath, std::ios::app);  // Append mode
        if (!outFile) {
            NS_LOG_UNCOND("Error opening file for appending.");
            return;
        }

        // Write header only if the file is empty
        outFile.seekp(0, std::ios::end);
        if (outFile.tellp() == 0) {

            if (experiment == "experiment1")
            {
                outFile << "Topology Size," << "Routing Protocol," << "Packet Delivery Ratio (%)," << "Average End-to-End Delay (s)," << "Throuput (bytes/second)," << "\n";
            }
            else 
            {
                outFile << "Grid Size," << "Routing Protocol," << "Packet Delivery Ratio (%)," << "Average End-to-End Delay (s)," << "Throuput (bytes/second)," << "\n";
            }
        }

        outFile << topology_or_grid_size << ", " << routingProtocol << ", "  << pdr << ", " << avgEndToEndDelay << ", " << throughput << "\n";
        outFile.close();

        NS_LOG_UNCOND("Results appended to " << csv_filepath);
    }

    // once a responder is notified of an emergency call, it should move towards the emergency caller
    void UrbanManetRoutingExperiment::MoveResponderToCaller(Ptr<Node> caller_node, Ptr<Node> responder_node, int &gridSize) 
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
            r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));  // stop the responder at the same coordinates as the caller
            return;
        }

        else 
        {
            // if the responder is out of bounds, it should stop immediately 
            if (r_mobility->GetPosition().x > gridSize || r_mobility->GetPosition().x < 0 || r_mobility->GetPosition().y > gridSize || r_mobility->GetPosition().y < 0) // if node goes out of bounds
            {
                r_mobility->SetVelocity(Vector3D(0.0, 0.0, 0.0));
                return;
            }

            // move the responder towards the caller 
            else 
            {
                r_mobility->SetVelocity(velocity);
                Simulator::Schedule(Seconds(1.0), &UrbanManetRoutingExperiment::MoveResponderToCaller, this, caller_node, responder_node, gridSize);
            }
        }
    }


    // Determine Emergency Centre / Emergency Responder that is closest to caller
    Ptr<Node> UrbanManetRoutingExperiment::NearestNodeToCaller(Ptr<Node> node, NodeContainer container) {
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
    void UrbanManetRoutingExperiment::FreezeNode(Ptr<Node> node, double interval)
    {
        Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
        mob->SetPosition(mob->GetPosition());

        Simulator::Schedule(Seconds(interval), &UrbanManetRoutingExperiment::FreezeNode, this, node, interval);
    }


    // Receive packet functions 
    Ptr<Socket> UrbanManetRoutingExperiment::RecvSocketConfig (Ptr<Node> node)
    {
        Ptr<Socket> recvSocket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 8080);
        recvSocket->Bind(local);  // Bind to port 8080
        return recvSocket;
    }

    void UrbanManetRoutingExperiment::ReceivePacket(Ptr<Socket> socket) {
        Ptr<Packet> packet;
        while ((packet = socket->Recv())) {  // Receive the packet
            uint8_t buffer[1024];  // Buffer to store received data
            packet->CopyData(buffer, packet->GetSize());
            packetReceiveTimes.push_back((Simulator::Now()).GetSeconds());
            bytesTotal += packet->GetSize();
            NS_LOG_UNCOND("Time = " << (Simulator::Now()).GetSeconds() << ": Packet received. " 
                                << std::string((char*)buffer, packet->GetSize()) << std::endl);
        }
    }


    // Send packet function 
    void UrbanManetRoutingExperiment::SendPacket(Ptr<Socket> socket, Ipv4Address destination, uint16_t port, std::string msg) {
        Ptr<Packet> packet = Create<Packet>((uint8_t*)msg.c_str(), msg.size());
        socket->SendTo(packet, 0, InetSocketAddress(destination, port)); // Send packet to receiver
        bytesTotal += packet->GetSize();
        NS_LOG_UNCOND("Time = " << (Simulator::Now()).GetSeconds() << ": Packet Sent. " << msg);

    }


    /*Function that configures all of the communication within an urban mobile ad-hoc network as follows:  
        1. Emergency Caller notifies the closest Emergency Centre of the emergency
        2. The Centre notifies the Emergency Responder that is closest to the caller 
        3. The Emergency responder node moves towards the node that made the initial emergency call*/ 
    
    void UrbanManetRoutingExperiment::EmergencyCallWithResponse(Ptr<Node> caller, NodeContainer &centre_container, NodeContainer &responder_container, UrbanManet &manet, int &gridSize)
    {
        // configure the emergency call 
        Ptr<Node> nearestCentre = NearestNodeToCaller(caller, centre_container);
        Ipv4Address centre_address = manet.get_adhocInterface().GetAddress(nearestCentre->GetId());

        Ptr<Node> nearestResponder = NearestNodeToCaller(caller, responder_container);
        Ipv4Address responder_address = manet.get_adhocInterface().GetAddress(nearestResponder->GetId());

        std::cout << "\nCall by Caller Node: " << caller->GetId() << " Centre: " << nearestCentre->GetId() << " Responder: " << nearestResponder->GetId() << std::endl;


        Ptr<Socket> recv_emergency_call = RecvSocketConfig(nearestCentre);
        recv_emergency_call->SetRecvCallback(MakeCallback(&UrbanManetRoutingExperiment::ReceivePacket, this));
        Ptr<Socket> send_emergency_call = Socket::CreateSocket(caller, UdpSocketFactory::GetTypeId());  // Sender

        std::string message = "Emergency call by node " + std::to_string(send_emergency_call->GetNode()->GetId()) 
        + " to node " +  std::to_string(recv_emergency_call->GetNode()->GetId()) 
        + ". At location " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
        + ", " + std::to_string(send_emergency_call->GetNode()->GetObject<MobilityModel>()->GetPosition().y);

        Simulator::Schedule(Seconds(0.0), &UrbanManetRoutingExperiment::SendPacket, this, send_emergency_call, centre_address, 8080, message);

        // configure the notification to the emergency responder 
        Ptr<Socket> recv_centre_req = RecvSocketConfig(nearestResponder);
        recv_centre_req->SetRecvCallback(MakeCallback(&UrbanManetRoutingExperiment::ReceivePacket, this));
        Ptr<Socket> send_centre_req = Socket::CreateSocket(nearestCentre, UdpSocketFactory::GetTypeId());  // Sender

        std::string message1 = "Centre Response by node " + std::to_string(send_centre_req->GetNode()->GetId())
        + " to node " +  std::to_string(recv_centre_req->GetNode()->GetId()) 
        + ". At location " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().x)
        + ", " + std::to_string(send_centre_req->GetNode()->GetObject<MobilityModel>()->GetPosition().y);

        Simulator::Schedule(Seconds(1.0), &UrbanManetRoutingExperiment::SendPacket, this, send_centre_req, responder_address, 8080, message1); 

        // move the responder to the caller
        Simulator::Schedule(Seconds(3.0), &UrbanManetRoutingExperiment::MoveResponderToCaller, this, caller, nearestResponder, gridSize); 
    } 


    // To make a caller node disappear from the animation after responder has attended it 
    void UrbanManetRoutingExperiment::ChangeNodeSize(AnimationInterface *anim, uint32_t nodeId, double newSize) 
    {
        anim->UpdateNodeSize(nodeId, newSize, newSize); // Width and height
    }

    // function to run the entire experiment
    void UrbanManetRoutingExperiment::RunExperiment(std::string experiment, int topologySize, int gridSize, std::string protocolName, double txp, std::string packetSize, std::string rate, std::string phyMode) 
    {
        bytesTotal = 0;

        // configure topology parameters 
        // int gridSize = 2 * topologySize;
        int numCivillians = topologySize * 0.7;
        int numCentres =  topologySize * 0.1; 
        int numResponders =  topologySize * 0.2;

        int numEmergencyCallers = topologySize * 0.3;

        double simulationTime = static_cast<double> (topologySize * 3);
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

        civillian_nodes.add_nodes(civillian_coords, gridSize);
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
        
        std::string animation_path_name;
        if (experiment == "experiment1") 
        {
            animation_path_name = "scratch/" + experiment + "/" + protocolName + "-manet-experiment-" + std::to_string(topologySize) + ".xml";
        }
        else 
        {
            animation_path_name = "scratch/" + experiment + "/" + protocolName + "-manet-experiment-" + std::to_string(gridSize) + ".xml";
        }
        // std::string animation_path_name = "scratch/" + experiment + "/" + protocolName + "-manet-experiment-" + std::to_string(topologySize) + ".xml";
        AnimationInterface anim (animation_path_name);
        anim.SetMaxPktsPerTraceFile(1000000000000000000);
        anim.EnablePacketMetadata(true);

        // change colour and size of Emergency Centre nodes to distinguish it from other nodes
        for (auto j = centre_container.Begin(); j != centre_container.End(); ++j)
        {
            Ptr<Node> object = *j;
            anim.UpdateNodeColor (*j, 0, 255, 0); // Green
            anim.UpdateNodeSize (*j, 5, 5);
        }

        // change colour and size of Emergency Responder nodes to distinguish it from other nodes
        for (auto j = responder_container.Begin(); j != responder_container.End(); ++j)
        {
            Ptr<Node> object = *j;
            anim.UpdateNodeColor (*j, 0, 0, 255); // Blue
            anim.UpdateNodeSize (*j, 2, 2);
        }
    
        double time_value = 2.0;

        for (int i=0; i<numEmergencyCallers; i++)  // emergency simulation for civillains that experience emergency
        {
            uint32_t caller_index = emergency_callers.Get(i)->GetId() - numCentres;
            uint32_t responder_index = NearestNodeToCaller(civillian_container.Get(caller_index), responder_container)->GetId() - numCivillians - numCentres;

            Ptr<RandomWaypointMobilityModel> caller_mobility = civillian_container.Get(caller_index)->GetObject<RandomWaypointMobilityModel>();
            Ptr<ConstantVelocityMobilityModel> responder_mobility = responder_container.Get(responder_index)->GetObject<ConstantVelocityMobilityModel>();

            Simulator::Schedule(Seconds(time_value), &UrbanManetRoutingExperiment::FreezeNode, this, civillian_container.Get(caller_index), 1.0);

            anim.UpdateNodeColor (civillian_container.Get(caller_index), 2, 2, 2); // Black indicates that the nodes experience emergency

            Simulator::Schedule(Seconds(time_value + 1.0), &UrbanManetRoutingExperiment::EmergencyCallWithResponse, this, civillian_container.Get(caller_index), centre_container, responder_container, manet, gridSize); 

            // Remove emergency caller that has been attended to
            Simulator::Schedule(Seconds(time_value + 6.0), &UrbanManetRoutingExperiment::ChangeNodeSize, this, &anim, civillian_container.Get(caller_index)->GetId(), 0); 

            time_value = time_value + 10;
        }


        Simulator::Stop(Seconds(simulationTime));

        // Setup flow monitor  
        Ptr<FlowMonitor> flowMonitor;
        FlowMonitorHelper flowHelper;
        flowMonitor = flowHelper.InstallAll();

        Simulator::Run();
        // std::cout << "Simulation Time:  " << Simulator::Now().GetSeconds() << std::endl;
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

        double throughput = static_cast<double>(bytesTotal) / simulationTime;


        NS_LOG_UNCOND("Overall Packet Delivery Ratio: " << pdr << "%");
        NS_LOG_UNCOND("Overall Avg End-to-End Delay: " << avgEndToEndDelay << " seconds");
        NS_LOG_UNCOND("Throughput: " << throughput << " bytes/sec");

        // Insert this function call before the end of ManetExperiment()
        if (experiment == "experiment1")
        {
            WriteResultsToCsv(experiment, topologySize, protocolName, pdr, avgEndToEndDelay, throughput);
        }
        else
        {
            WriteResultsToCsv(experiment ,gridSize, protocolName, pdr, avgEndToEndDelay, throughput);
        }

        flowMonitor->SerializeToXmlFile("flow-monitor-results.xml", true, true);
    }

}
