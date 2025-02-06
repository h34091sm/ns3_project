#include "EmergencyCentre.h"

namespace ns3
{

/* ... */


void EmergencyCentre::add_nodes(std::vector<ns3::Vector3D> & nodes_coordinates)
{
    container.Create(nodes_coordinates.size());
    
    for (size_t i=0; i<nodes_coordinates.size(); i++)
    {
        list_position_alloc->Add(nodes_coordinates[i]);
    }

    centre_mobility.SetPositionAllocator(list_position_alloc);
    centre_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    centre_mobility.Install(container);
}

ns3::NodeContainer EmergencyCentre::get_container()
{
    return container;
}

}
 