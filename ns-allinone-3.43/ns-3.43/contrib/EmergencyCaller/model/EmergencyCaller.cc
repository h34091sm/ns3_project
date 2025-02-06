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
    caller_mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (0, 100, 0, 100)));    
    caller_mobility.Install(container);


    for (size_t i = 0; i < node_coordinates.size(); ++i)
    {
        Ptr<Node> node = container.Get(i);
        Ptr<RandomWalk2dMobilityModel > mobility = node->GetObject<RandomWalk2dMobilityModel >();
        // node->AggregateObject(mobility);
        mobility->SetPosition(node_coordinates[i]);
    }
}

}
