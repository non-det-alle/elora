#include "utilities.cc"

#include "ns3/base-end-device-lorawan-mac.h"
#include "ns3/building-allocator.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/buildings-helper.h"
#include "ns3/callback.h"
#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/double.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/forwarder-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-helper.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/mobility-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/pointer.h"
#include "ns3/position-allocator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE_EXAMPLE_WITH_UTILITIES("AlohaThroughput");

// Network settings
int nDevices = 200;
int nGateways = 1;
double radius = 1000;
double simulationTime = 100;

// Channel model
bool realisticChannelModel = false;

int
main(int argc, char* argv[])
{
    std::string interferenceMatrix = "ALOHA";

    CommandLine cmd;
    cmd.AddValue("nDevices", "Number of end devices to include in the simulation", nDevices);
    cmd.AddValue("simulationTime", "Simulation Time", simulationTime);
    cmd.AddValue("interferenceMatrix",
                 "Interference matrix to use [ALOHA, GOURSAUD]",
                 interferenceMatrix);
    cmd.AddValue("radius", "Radius of the deployment", radius);
    cmd.Parse(argc, argv);

    int appPeriodSeconds = simulationTime;

    // Set up logging
    LogComponentEnable("AlohaThroughput", LOG_LEVEL_ALL);

    // Make all devices use SF7 (i.e., DR5)
    // Config::SetDefault ("ns3::BaseEndDeviceLorawanMac::DataRate", UintegerValue (5));

    /***********
     *  Setup  *
     ***********/

    // Mobility
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator",
                                  "rho",
                                  DoubleValue(radius),
                                  "X",
                                  DoubleValue(0.0),
                                  "Y",
                                  DoubleValue(0.0));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    /************************
     *  Create the channel  *
     ************************/

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    if (realisticChannelModel)
    {
        // Create the correlated shadowing component
        Ptr<CorrelatedShadowingPropagationLossModel> shadowing =
            CreateObject<CorrelatedShadowingPropagationLossModel>();

        // Aggregate shadowing to the logdistance loss
        loss->SetNext(shadowing);

        // Add the effect to the channel propagation loss
        Ptr<BuildingPenetrationLoss> buildingLoss = CreateObject<BuildingPenetrationLoss>();

        shadowing->SetNext(buildingLoss);
    }

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetInterference("IsolationMatrix", EnumValue(sirMap.at(interferenceMatrix)));
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();
    macHelper.SetRegion(LorawanMacHelper::ALOHA);

    // Create the LorawanHelper
    LorawanHelper helper = LorawanHelper();
    helper.EnablePacketTracking(); // Output filename

    // Create the NetworkServerHelper
    NetworkServerHelper nsHelper = NetworkServerHelper();

    // Create the ForwarderHelper
    ForwarderHelper forHelper = ForwarderHelper();

    /************************
     *  Create End Devices  *
     ************************/

    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create(nDevices);

    // Assign a mobility model to each node
    mobility.Install(endDevices);

    // Make it so that nodes are at a certain height > 0
    for (auto j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel>();
        Vector position = mobility->GetPosition();
        position.z = 1.2;
        mobility->SetPosition(position);
    }

    // Create the LoraNetDevices of the end devices
    uint8_t nwkId = 54;
    uint32_t nwkAddr = 1864;
    Ptr<LoraDeviceAddressGenerator> addrGen =
        CreateObject<LoraDeviceAddressGenerator>(nwkId, nwkAddr);

    // Create the LoraNetDevices of the end devices
    macHelper.SetAddressGenerator(addrGen);
    phyHelper.SetType("ns3::EndDeviceLoraPhy");
    macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
    helper.Install(phyHelper, macHelper, endDevices);

    // Now end devices are connected to the channel

    // Connect trace sources
    for (auto j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<Node> node = *j;
        Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(node->GetDevice(0));
        Ptr<LoraPhy> phy = loraNetDevice->GetPhy();
    }

    /*********************
     *  Create Gateways  *
     *********************/

    // Create the gateway nodes (allocate them uniformely on the disc)
    NodeContainer gateways;
    gateways.Create(nGateways);

    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    // Make it so that nodes are at a certain height > 0
    allocator->Add(Vector(0.0, 0.0, 15.0));
    mobility.SetPositionAllocator(allocator);
    mobility.Install(gateways);

    // Create a netdevice for each gateway
    phyHelper.SetType("ns3::GatewayLoraPhy");
    macHelper.SetType("ns3::GatewayLorawanMac");
    helper.Install(phyHelper, macHelper, gateways);

    NS_LOG_DEBUG("Completed configuration");

    /*********************************************
     *  Install applications on the end devices  *
     *********************************************/

    Time appStopTime = Seconds(simulationTime);
    int packetSize = 50;
    PeriodicSenderHelper appHelper = PeriodicSenderHelper();
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    appHelper.SetPacketSize(packetSize);
    ApplicationContainer appContainer = appHelper.Install(endDevices);

    appContainer.Start(Seconds(0));
    appContainer.Stop(appStopTime);

    std::ofstream outputFile;
    // Delete contents of the file as it is opened
    outputFile.open("durations.txt", std::ofstream::out | std::ofstream::trunc);
    for (uint8_t sf = 7; sf <= 12; sf++)
    {
        LoraPhyTxParameters txParams;
        txParams.sf = sf;
        txParams.headerDisabled = false;
        txParams.codingRate = 1;
        txParams.bandwidthHz = 125000;
        txParams.nPreamble = 8;
        txParams.crcEnabled = true;
        txParams.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(txParams) > MilliSeconds(16);
        Ptr<Packet> pkt = Create<Packet>(packetSize);

        LoraFrameHeader fHdr = LoraFrameHeader();
        fHdr.SetAsUplink();
        fHdr.SetFPort(1);
        fHdr.SetAddress(LoraDeviceAddress());
        fHdr.SetAdr(false);
        fHdr.SetAdrAckReq(false);
        fHdr.SetFCnt(0);
        pkt->AddHeader(fHdr);

        LorawanMacHeader mHdr = LorawanMacHeader();
        mHdr.SetFType(ns3::lorawan::LorawanMacHeader::UNCONFIRMED_DATA_UP);
        mHdr.SetMajor(1);
        pkt->AddHeader(mHdr);

        outputFile << LoraPhy::GetTimeOnAir(pkt, txParams).GetMicroSeconds() << " ";
    }
    outputFile.close();

    /**************************
     *  Create Network Server  *
     ***************************/

    // Create the NS node
    NodeContainer networkServer;
    networkServer.Create(1);

    // PointToPoint links between gateways and server
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    for (auto gw = gateways.Begin(); gw != gateways.End(); ++gw)
    {
        p2p.Install(networkServer.Get(0), *gw);
    }

    // Create a NS for the network
    nsHelper.SetEndDevices(endDevices);
    nsHelper.Install(networkServer);

    // Create a forwarder for each gateway
    forHelper.Install(gateways);

    // Install trace sources
    for (auto node = gateways.Begin(); node != gateways.End(); node++)
    {
        DynamicCast<LoraNetDevice>((*node)->GetDevice(0))
            ->GetPhy()
            ->TraceConnectWithoutContext("ReceivedPacket", MakeCallback(OnPacketReceptionCallback));
    }

    // Install trace sources
    for (auto node = endDevices.Begin(); node != endDevices.End(); node++)
    {
        DynamicCast<LoraNetDevice>((*node)->GetDevice(0))
            ->GetPhy()
            ->TraceConnectWithoutContext("StartSending", MakeCallback(OnTransmissionCallback));
    }

    LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    ////////////////
    // Simulation //
    ////////////////

    Simulator::Stop(appStopTime + Hours(1));

    NS_LOG_INFO("Running simulation...");
    Simulator::Run();

    Simulator::Destroy();

    /////////////////////////////
    // Print results to stdout //
    /////////////////////////////
    NS_LOG_INFO("Computing performance metrics...");

    for (int i = 0; i < 6; i++)
    {
        std::cout << packetsSent.at(i) << " " << packetsReceived.at(i) << std::endl;
    }

    return 0;
}
