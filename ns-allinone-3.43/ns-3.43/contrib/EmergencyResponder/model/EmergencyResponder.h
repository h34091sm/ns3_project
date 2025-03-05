#ifndef EMERGENCYRESPONDER_H
#define EMERGENCYRESPONDER_H

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup EmergencyResponder Description of the EmergencyResponder
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

#include <vector>

namespace ns3
{

// Each class should be documented using Doxygen,
// and have an \ingroup EmergencyResponder directive

/* ... */
class EmergencyResponder
{
    private:
    ns3::NodeContainer container;
    ns3::MobilityHelper responder_mobility;
    ns3::Ptr<ListPositionAllocator> list_position_alloc = CreateObject<ListPositionAllocator> ();

    public:
    void add_nodes(std::vector<ns3::Vector3D> & nodes_coordinates);
    ns3::NodeContainer get_container() {return container;}

};

}

#endif /* EMERGENCYRESPONDER_H */
