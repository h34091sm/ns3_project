/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/core-module.h"
#include "ns3/EmergencyCentre.h"
#include "ns3/EmergencyCaller.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ScratchSimulator");

int
main(int argc, char* argv[])
{
    NS_LOG_UNCOND("Scratch Simulator");

    std::vector<ns3::Vector3D> centre_coords = {ns3::Vector3D(43.0, 20.0, 0.0), ns3::Vector3D(10.0, 20.0, 0.0), ns3::Vector3D(30.0, 20.0, 0.0)};
    ns3::EmergencyCentre emergency_centre;
    emergency_centre.add_nodes(centre_coords);
    NodeContainer centre_container = emergency_centre.get_container();


    std::vector<ns3::Vector3D> caller_coords = {ns3::Vector3D(43.0, 20.0, 0.0), ns3::Vector3D(10.0, 20.0, 0.0), ns3::Vector3D(30.0, 20.0, 0.0)};
    ns3::EmergencyCaller emergency_caller;
    emergency_caller.add_nodes(caller_coords);
    NodeContainer caller_container = emergency_caller.get_container();

    

    // change colour and size of nodes
    AnimationInterface anim ("scratch-simulator.xml");

    for (auto j = centre_container.Begin(); j != centre_container.End(); ++j)
    {
        Ptr<Node> object = *j;
        anim.UpdateNodeColor (*j, 0, 255, 0); // Green
        anim.UpdateNodeSize (*j, 2, 2);
    }



    // Simulator::
    Simulator::Stop(Seconds(50.0)); 
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}



// Add implementation of EmergencyResponder and EmergencyCaller