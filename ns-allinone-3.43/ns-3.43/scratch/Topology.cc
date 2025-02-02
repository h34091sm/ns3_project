#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
// #include "project_header_files/EmergencyCentre.h"

// C++ imports
#include <vector>

// create 2 emergency centre static node, 2 first responder dynamic emergency_centre and 3 emergency caller emergency_centre 
// create p2p channel between emergency centre with callers and responders 

using namespace ns3;
using namespace std;


class Topology {
  private: 
    NodeContainer emergency_centre; 
    NodeContainer emergency_responder; 
    NodeContainer emergency_caller;

    MobilityHelper centre_mobility;
    MobilityHelper responder_mobility;
    MobilityHelper caller_mobility;

    Ptr<ListPositionAllocator> list_position_alloc = CreateObject<ListPositionAllocator> ();

  public:
    void setup_centre(int num_nodes, std::vector<ns3::Vector3D>& node_coordinates)
    {
      emergency_centre.Create(num_nodes);

      for (size_t i=0; i<num_nodes; i++) 
      {
        list_position_alloc->Add(node_coordinates[i]);
      }

      centre_mobility.SetPositionAllocator(list_position_alloc);
      centre_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      centre_mobility.Install(emergency_centre);
    }

    NodeContainer get_centre()
    {
      return emergency_centre;
    }

    void setup_responder(int num_nodes, std::vector<ns3::Vector3D>& node_coordinates, std::vector<ns3::Vector3D>& node_velocities)
    {
      emergency_responder.Create(num_nodes);
      for (size_t i=0; i<num_nodes; i++) 
      {
        list_position_alloc->Add(node_coordinates[i]);
      }

      centre_mobility.SetPositionAllocator(list_position_alloc);
      centre_mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
      centre_mobility.Install(emergency_responder);

      for (size_t i=0; i<num_nodes; i++) 
      {
        list_position_alloc->Add(node_coordinates[i]);
      }


      for (size_t i = 0; i < num_nodes; ++i)
      {
        Ptr<Node> node = emergency_responder.Get(i);
        Ptr<ConstantVelocityMobilityModel> mobility = node->GetObject<ConstantVelocityMobilityModel>();
        mobility->SetPosition(node_coordinates[i]);
        mobility->SetVelocity(Vector(node_velocities[i].x, node_velocities[i].y, node_velocities[i].z));
      }
    }

    NodeContainer get_responder()
    {
      return emergency_responder;
    }


  

};

// int main (int argc, char *argv[])
// {
//   CommandLine cmd;
//   cmd.Parse (argc, argv);

//   // Topology top1;
//   // std::vector<ns3::Vector3D> centre_coords = {ns3::Vector3D(50.0, 50.0, 0.0), ns3::Vector3D(43.0, 50.0, 0.0)};
//   // top1.setup_centre(2, centre_coords);

//   // std::vector<ns3::Vector3D> responder_coords = {ns3::Vector3D(35.0, 35.0, 0.0), ns3::Vector3D(43.0, 50.0, 0.0)};
//   // std::vector<ns3::Vector3D> responder_velocities = {ns3::Vector3D(0.0, -20.0, 0.0), ns3::Vector3D(-10.0, 10.0, 0.0)};
//   // top1.setup_responder(2, responder_coords, responder_velocities);

//   // NodeContainer emergency_centre = top1.get_centre();
//   // AnimationInterface anim ("Topology.xml");


//   // for (auto j = emergency_centre.Begin(); j != emergency_centre.End(); ++j)
//   // {
//   //   Ptr<Node> object = *j;
//   //   anim.UpdateNodeColor (*j, 0, 255, 0); // Green
//   //   anim.UpdateNodeSize (*j, 2, 2);
//   // }

//   std::vector<ns3::Vector3D> centre_coords = {ns3::Vector3D(50.0, 50.0, 0.0), ns3::Vector3D(43.0, 50.0, 0.0)};

//   EmergencyCentre centre(centre_coords);
//   NodeContainer emergency_container = centre.get_container();
//   AnimationInterface anim ("Topology.xml");

//   for (auto j = emergency_container.Begin(); j != emergency_container.End(); ++j)
//   {
//     Ptr<Node> object = *j;
//     anim.UpdateNodeColor (*j, 0, 255, 0); // Green
//     anim.UpdateNodeSize (*j, 2, 2);
//   }

//   Simulator::Stop(Seconds(10.0)); // Run the simulation for 10 seconds
//   Simulator::Run ();
//   Simulator::Destroy ();

//   return 0;
// }

int main ()
{
  return 0;
}