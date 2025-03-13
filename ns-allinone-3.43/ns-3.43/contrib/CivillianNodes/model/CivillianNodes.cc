#include "CivillianNodes.h"

namespace ns3
{

/* ... */

void CivillianNodes::add_nodes(std::vector<ns3::Vector3D>& node_coordinates, int gridSize)
{
    container.Create(node_coordinates.size());
    for (size_t i=0; i<node_coordinates.size(); i++) 
    {
        list_position_alloc->Add(node_coordinates[i]);
    }

    caller_mobility.SetPositionAllocator(list_position_alloc);

      // Set up a RandomRectanglePositionAllocator for destination selection
    Ptr<RandomRectanglePositionAllocator> randomAlloc = CreateObject<RandomRectanglePositionAllocator> ();

    std::string str_gridSize = "ns3::UniformRandomVariable[Min=0.0|Max=" + std::to_string(gridSize) + "]";
    
    randomAlloc->SetAttribute("X", StringValue(str_gridSize));
    randomAlloc->SetAttribute("Y", StringValue(str_gridSize));

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
