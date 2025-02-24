#include "EmergencyCaller.h"

namespace ns3
{

/* ... */

void EmergencyCaller::add_nodes(std::vector<ns3::Vector3D>& node_coordinates)
{
    container.Create(node_coordinates.size());
    for (size_t i=0; i<node_coordinates.size(); i++) 
    {
        list_position_alloc->Add(node_coordinates[i]);
    }

    caller_mobility.SetPositionAllocator(list_position_alloc);

      // Set up a RandomRectanglePositionAllocator for destination selection
    Ptr<RandomRectanglePositionAllocator> randomAlloc = CreateObject<RandomRectanglePositionAllocator> ();
    randomAlloc->SetAttribute("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    randomAlloc->SetAttribute("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));

    caller_mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                     "Speed", StringValue("ns3::ConstantRandomVariable[Constant=10.0]"),
                                      // "Pause", StringValue("ns3::ConstantRandomVariable[Constant=50.0]"),
                                     "PositionAllocator", PointerValue(randomAlloc));    
    caller_mobility.Install(container);


    for (size_t i = 0; i < node_coordinates.size(); ++i)
    {
        Ptr<Node> node = container.Get(i);
        Ptr<RandomWaypointMobilityModel > mobility = node->GetObject<RandomWaypointMobilityModel>();
        // node->AggregateObject(mobility);
        mobility->SetPosition(node_coordinates[i]);
    }
}

}
