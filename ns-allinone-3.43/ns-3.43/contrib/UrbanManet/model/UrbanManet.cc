#include "UrbanManet.h"

namespace ns3
{

    UrbanManet::UrbanManet(std::string &protocolName, double &txp, std::string &packetSize, std::string &rate, std::string &phyMode, NodeContainer &nodes)
    {

        Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue(packetSize));
        Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue(rate));
    
        // Set Non-unicastMode rate to unicast mode
        Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

        // set ting up wifi phy and channel using helpers
        WifiHelper wifi;
        wifi.SetStandard(WIFI_STANDARD_80211b);

        YansWifiPhyHelper wifiPhy;
        YansWifiChannelHelper wifiChannel;
        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
        wifiPhy.SetChannel(wifiChannel.Create());

        // Add a mac and disable rate control
        WifiMacHelper wifiMac;
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                    "DataMode",
                                    StringValue(phyMode),
                                    "ControlMode",
                                    StringValue(phyMode));

        wifiPhy.Set("TxPowerStart", DoubleValue(txp));
        wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

        wifiMac.SetType("ns3::AdhocWifiMac");
        
        NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac, nodes);
            ////////// Ignore? 
        MobilityHelper mobilityAdhoc;
        int64_t streamIndex = 0; // used to get consistent mobility across scenarios

        ObjectFactory pos;
        pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
        pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=300.0]"));
        pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1500.0]"));

        Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
        streamIndex += taPositionAlloc->AssignStreams(streamIndex);


        AodvHelper aodv;
        OlsrHelper olsr;
        DsdvHelper dsdv;
        DsrHelper dsr;
        DsrMainHelper dsrMain;
        Ipv4ListRoutingHelper list;
        InternetStackHelper internet;

        if (protocolName == "OLSR")
        {
            list.Add(olsr, 100);
            internet.SetRoutingHelper(list);
            internet.Install(nodes);
        }
        else if (protocolName == "AODV")
        {
            list.Add(aodv, 100);
            internet.SetRoutingHelper(list);
            internet.Install(nodes);
        }
        else if (protocolName == "DSDV")
        {
            list.Add(dsdv, 100);
            internet.SetRoutingHelper(list);
            internet.Install(nodes);
        }
        else
        {
            NS_FATAL_ERROR("No such protocol:" << protocolName);

        }

        Ipv4AddressHelper addressAdhoc;
        addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
        this->adhocInterfaces = addressAdhoc.Assign(adhocDevices);
    
        OnOffHelper onoff1("ns3::UdpSocketFactory", Address());
        onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
        onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
    }



/* ... */

}
