/*
 * This program creates an elora network with nodes implementing bicycle mobility.
 */

// ns3 imports
#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/tap-bridge-helper.h"

// lorawan imports
#include "ns3/bike-application-helper.h"
#include "ns3/bike-sharing-mobility-helper.h"
#include "ns3/chirpstack-helper.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/lorawan-helper.h"
#include "ns3/udp-forwarder-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("BikeMobilityExample");
#include "../utilities.cc"

/* Global declaration of connection helper for signal handling */
ChirpstackHelper csHelper;

int
main(int argc, char* argv[])
{
    /***************************
     *  Simulation parameters  *
     ***************************/

    std::string tenant = "ELoRa";
    std::string apiAddr = "127.0.0.1";
    uint16_t apiPort = 8090;
    std::string token =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9."
        "eyJhdWQiOiJjaGlycHN0YWNrIiwiaXNzIjoiY2hpcnBzdGFjayIsInN1YiI6IjZlMjQ4NjljLWQxMjItNDZkOS04Nz"
        "E0LTM5Yzc4Nzg4OTRhZCIsInR5cCI6ImtleSJ9.IB20o6Jrcwj5qZ9mPEuthzzqMyc3YNSl8by_ZXrjqhw";
    uint16_t destPort = 1700;

    int periods = 24 * 31; // Hours
    int gatewayRings = 2;
    double range = 2426.85; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
    int nDevs = -1;
    std::string sir = "GOURSAUD";

    std::string filepath = "None";

    /* Expose parameters to command line */
    {
        CommandLine cmd(__FILE__);
        cmd.AddValue("tenant", "Chirpstack tenant name of this simulation", tenant);
        cmd.AddValue("apiAddr", "Chirpstack REST API endpoint IP address", apiAddr);
        cmd.AddValue("apiPort", "Chirpstack REST API endpoint IP address", apiPort);
        cmd.AddValue("token", "Chirpstack API token (to be generated in Chirpstack UI)", token);
        cmd.AddValue("destPort", "Port used by the Chirpstack Gateway Bridge", destPort);
        cmd.AddValue("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
        cmd.AddValue("devs", "Number of devices to simulate (-1 defaults to all)", nDevs);
        cmd.AddValue("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
        cmd.AddValue("range", "Radius of the device allocation disk around a gateway)", range);
        cmd.AddValue("sir", "Signal to Interference Ratio matrix used for interference", sir);
        cmd.AddValue("adr", "ns3::BaseEndDeviceLorawanMac::ADRBit");
        cmd.AddValue(
            "data",
            "Complete path to the dataset containing the bike trips. The file should be a csv "
            "file, with one bike trip per line. It must contain start time, end time, starting X "
            "position, starting Y position, ending X position, ending Y position, and a bike ID. "
            "Timestamps are in seconds, X/Y coordinates are in meters, and the ID is any string. "
            "The delimiter is a comma ',' and the first line will be always skipped (usually has "
            "column names). It is up to you to provide trips that are sorted by start time and "
            "that do not overlap in time for the same bike.",
            filepath);
        cmd.Parse(argc, argv);
        NS_ASSERT((periods >= 0) and (nDevs >= -1) and (gatewayRings > 0));
    }

    /* Apply global configurations */
    {
        ///////////////// Real-time operation, necessary to interact with the outside world.
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
        GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
        Config::SetDefault("ns3::BaseEndDeviceLorawanMac::ADRBackoff", BooleanValue(true));
        Config::SetDefault("ns3::BaseEndDeviceLorawanMac::EnableCryptography", BooleanValue(true));
        Config::SetDefault("ns3::AdrComponent::SNRDeviceMargin",
                           DoubleValue(10 * log10(-1 / log(0.98))));
        ///////////////// Needed to manage the variance introduced by real world interaction
        Config::SetDefault("ns3::ClassAEndDeviceLorawanMac::RecvWinSymb", UintegerValue(16));
    }

    /* Logging options */
    {
        //!> Requirement: build ns3 with debug option
        // LogComponentEnable("BaseEndDeviceLorawanMac", LOG_LEVEL_DEBUG);
        // LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_INFO);
        // LogComponentEnable("BikeSharingMobilityHelper", LOG_LEVEL_DEBUG);
        // LogComponentEnable("BikeApplication", LOG_LEVEL_DEBUG);
        LogComponentEnableAll(LOG_PREFIX_FUNC);
        LogComponentEnableAll(LOG_PREFIX_NODE);
        LogComponentEnableAll(LOG_PREFIX_TIME);
    }

    /******************
     *  Radio Channel *
     ******************/

    Ptr<LoraChannel> channel;
    {
        // Delay obtained from distance and speed of light in vacuum (constant)
        Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

        // Path loss take from experimental results in https://doi.org/10.1109/JIOT.2019.2953804
        auto loss = CreateObject<LogDistancePropagationLossModel>();
        loss->SetAttribute("Exponent", DoubleValue(2.75));
        loss->SetAttribute("ReferenceLoss", DoubleValue(74.85));
        loss->SetAttribute("ReferenceDistance", DoubleValue(1.0));
        auto shadowing = CreateObject<RandomPropagationLossModel>();
        shadowing->SetAttribute("Variable",
                                StringValue("ns3::NormalRandomVariable[Variance=" +
                                            std::to_string(std::exp2(11.25)) + "]"));
        loss->SetNext(shadowing);

        channel = CreateObject<LoraChannel>(loss, delay);
    }

    /**************
     *  Mobility  *
     **************/

    MobilityHelper mobilityGw;
    BikeSharingMobilityHelper mobilityEd;
    {
        // Gateway mobility
        mobilityGw.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // In hex tiling, distance = range * cos (pi/6) * 2 to have no holes
        double gatewayDistance = range * std::cos(M_PI / 6) * 2;
        auto hexAllocator = CreateObject<HexGridPositionAllocator>();
        hexAllocator->SetAttribute("Z", DoubleValue(30.0));
        hexAllocator->SetAttribute("distance", DoubleValue(gatewayDistance));
        mobilityGw.SetPositionAllocator(hexAllocator);

        // End Device mobility
        NS_ASSERT_MSG(filepath != "None", "Bike trips dataset file not provided.");
        mobilityEd.Add(filepath);
    }

    /******************
     *  Create Nodes  *
     ******************/

    Ptr<Node> exitnode;
    NodeContainer gateways;
    NodeContainer endDevices;
    {
        exitnode = CreateObject<Node>();

        int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
        gateways.Create(nGateways);
        mobilityGw.Install(gateways);

        endDevices.Create((nDevs == -1) ? mobilityEd.GetNBikes() : nDevs);
        mobilityEd.Install(endDevices);
    }

    /************************
     *  Create Net Devices  *
     ************************/

    /* Csma between gateways and tap-bridge (represented by exitnode) */
    {
        NodeContainer csmaNodes(NodeContainer(exitnode), gateways);

        // Connect the bridge to the gateways with csma
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
        csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
        csma.SetDeviceAttribute("Mtu", UintegerValue(1500));
        auto csmaNetDevs = csma.Install(csmaNodes);

        // Install and initialize internet stack on gateways and bridge nodes
        InternetStackHelper internet;
        internet.Install(csmaNodes);

        Ipv4AddressHelper addresses;
        addresses.SetBase("10.1.2.0", "255.255.255.0");
        addresses.Assign(csmaNetDevs);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    ///////////////// Attach a Tap-bridge to outside the simulation to the server csma device
    TapBridgeHelper tapBridge;
    tapBridge.SetAttribute("Mode", StringValue("ConfigureLocal"));
    tapBridge.SetAttribute("DeviceName", StringValue("ns3-tap"));
    tapBridge.Install(exitnode, exitnode->GetDevice(0));

    LorawanHelper loraHelper;
    {
        // Physiscal layer settings
        LoraPhyHelper phyHelper;
        phyHelper.SetInterference("IsolationMatrix", EnumValue(sirMap.at(sir)));
        phyHelper.SetChannel(channel);

        // Create a LoraDeviceAddressGenerator
        /////////////////// Enables full parallelism between ELoRa instances
        uint8_t nwkId = RngSeedManager::GetRun();
        auto addrGen = CreateObject<LoraDeviceAddressGenerator>(nwkId);

        // Mac layer settings
        LorawanMacHelper macHelper;
        macHelper.SetRegion(LorawanMacHelper::EU);
        macHelper.SetAddressGenerator(addrGen);

        // Create the LoraNetDevices of the gateways
        phyHelper.SetType("ns3::GatewayLoraPhy");
        macHelper.SetType("ns3::GatewayLorawanMac");
        loraHelper.Install(phyHelper, macHelper, gateways);

        // Create the LoraNetDevices of the end devices
        phyHelper.SetType("ns3::EndDeviceLoraPhy");
        macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
        loraHelper.Install(phyHelper, macHelper, endDevices);
    }

    /*************************
     *  Create Applications  *
     *************************/

    {
        // Install UDP forwarders in gateways
        UdpForwarderHelper forwarderHelper;
        forwarderHelper.SetAttribute("RemoteAddress", AddressValue(Ipv4Address("10.1.2.1")));
        forwarderHelper.SetAttribute("RemotePort", UintegerValue(destPort));
        forwarderHelper.Install(gateways);

        // Install applications in EDs
        BikeApplicationHelper bikeAppHelper;
        bikeAppHelper.SetAttribute("Interval", TimeValue(Minutes(2)));
        bikeAppHelper.SetAttribute("PacketSize", UintegerValue(12));
        bikeAppHelper.Install(endDevices);
    }

    /***************************
     *  Simulation and metrics *
     ***************************/

    ///////////////////// Signal handling
    OnInterrupt([](int signal) { csHelper.CloseConnection(signal); });
    ///////////////////// Register tenant, gateways, and devices on the real server
    csHelper.SetTenant(tenant);
    csHelper.InitConnection(apiAddr, apiPort, token);
    csHelper.Register(NodeContainer(endDevices, gateways));

#ifdef NS3_LOG_ENABLE
    // Print current configuration
    std::vector<int> nDevsPerSF(1, endDevices.GetN());
    PrintConfigSetup(endDevices.GetN(), range, gatewayRings, nDevsPerSF);
    loraHelper.EnableSimulationTimePrinting(Hours(24));
#endif // NS3_LOG_ENABLE

    Simulator::Stop(Hours(periods));

    // Start simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
