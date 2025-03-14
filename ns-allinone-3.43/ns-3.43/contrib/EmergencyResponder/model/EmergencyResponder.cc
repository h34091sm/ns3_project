#include "EmergencyResponder.h"

namespace ns3
{

/* ... */


// Given node coordinates, configure responder nodes using constant velocity model
void EmergencyResponder::add_nodes(std::vector<ns3::Vector3D>& node_coordinates)
{
    container.Create(node_coordinates.size());
    for (size_t i=0; i<node_coordinates.size(); i++) 
    {
        list_position_alloc->Add(node_coordinates[i]);
    }

    responder_mobility.SetPositionAllocator(list_position_alloc);
    responder_mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    responder_mobility.Install(container);


    for (size_t i = 0; i < node_coordinates.size(); ++i)
    {
        Ptr<ConstantVelocityMobilityModel> mobility = container.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        mobility->SetPosition(node_coordinates[i]); // Spaced 10m apart
        mobility->SetVelocity(Vector(0.0, 0.0, 0.0)); // Initially static
    }
}


}
