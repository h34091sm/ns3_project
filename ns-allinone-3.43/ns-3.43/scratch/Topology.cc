#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

// create 2 emergency centre static node, 2 first responder dynamic emergency_centre and 3 emergency caller emergency_centre 
// create p2p channel between emergency centre with callers and responders 

using namespace ns3;
using namespace std;

int main (int argc, char *argv[])

{

  CommandLine cmd;
  cmd.Parse (argc, argv);


  NodeContainer emergency_centre; 
  NodeContainer emergency_responder; 
  NodeContainer emergency_caller; 

// create nodes
  emergency_centre.Create(2);
  emergency_responder.Create(4);
  emergency_caller.Create(8);

  // setup mobility and grid 

  MobilityHelper centre_mobility;
  MobilityHelper responder_mobility;
  MobilityHelper caller_mobility;

 

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  // emergency centre nodes 
  positionAlloc->Add (Vector (40.0, 50.0, 0.0)); // Node 0 at (0,0,0)
  positionAlloc->Add (Vector (50.0, 50.0, 0.0)); // Node 1 at (10,0,0)


  positionAlloc->Add (Vector (38.0, 50.0, 0.0));
  positionAlloc->Add (Vector (42.0, 50.0, 0.0));
  positionAlloc->Add (Vector (48.0, 50.0, 0.0));
  positionAlloc->Add (Vector (52.0, 50.0, 0.0));


  centre_mobility.SetPositionAllocator(positionAlloc);
  centre_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  centre_mobility.Install(emergency_centre);

  responder_mobility.SetPositionAllocator(positionAlloc);
  responder_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  responder_mobility.Install(emergency_responder);

  // caller_mobility.SetPositionAllocator(positionAlloc);
  // caller_mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  // caller_mobility.Install(emergency_centre);

// print the position of emergency centre nodes 
  for (auto j = emergency_centre.Begin(); j != emergency_centre.End(); ++j)
  {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
      NS_ASSERT(position);
      Vector pos = position->GetPosition();
      std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
  }

  AnimationInterface anim ("Topology.xml");

  // Setting color of nodes
  for (auto j = emergency_centre.Begin(); j != emergency_centre.End(); ++j)
  {
    Ptr<Node> object = *j;
    anim.UpdateNodeColor (*j, 0, 255, 0); // Green
    anim.UpdateNodeSize (*j, 2, 2);
  }

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

  // std::cout << "hello" << std::endl;
}

