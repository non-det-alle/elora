/*
 * This program produces real-time traffic to a private instance of the things stack.
 * Key elements are preceded by a comment with lots of dashes ( ///////////// )
 */

#include "utilities.cc"

// ns3 imports
#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/tap-bridge-helper.h"

// lorawan imports
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/lorawan-helper.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/range-position-allocator.h"
#include "ns3/the-things-stack-helper.h"
#include "ns3/udp-forwarder-helper.h"
#include "ns3/urban-traffic-helper.h"

// cpp imports
#include <unordered_map>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE_EXAMPLE_WITH_UTILITIES("EloraTTSExample");

/* Global declaration of connection helper for signal handling */
TheThingsStackHelper ttsHelper;

int
main(int argc, char* argv[])
{
    /***************************
     *  Simulation parameters  *
     ***************************/

    std::string app = "ELoRa";
    std::string apiAddr = "127.0.0.1";
    uint16_t apiPort = 1885;
    std::string token = "...";

    uint16_t destPort = 1700;

    double periods = 24; // H * D
    int gatewayRings = 1;
    double range = 2540.25; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
    int nDevices = 1;
    std::string sir = "CROCE";
    bool initializeSF = true;
    bool testDev = false;
    bool file = false; // Warning: will produce a file for each gateway
    bool log = false;

    /* Expose parameters to command line */
    {
        CommandLine cmd(__FILE__);
        cmd.AddValue("app", "The Things Stack application name of this simulation", app);
        cmd.AddValue("apiAddr", "The Things Stack REST API endpoint IP address", apiAddr);
        cmd.AddValue("apiPort", "The Things Stack REST API endpoint IP address", apiPort);
        cmd.AddValue("token", "The Things Stack API token (to be generated in the UI)", token);
        cmd.AddValue("destPort", "Port used by the The Things Stack Gateway Server", destPort);
        cmd.AddValue("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
        cmd.AddValue("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
        cmd.AddValue("range", "Radius of the device allocation disk around a gateway)", range);
        cmd.AddValue("devices", "Number of end devices to include in the simulation", nDevices);
        cmd.AddValue("sir", "Signal to Interference Ratio matrix used for interference", sir);
        cmd.AddValue("initSF", "Whether to initialize the SFs", initializeSF);
        cmd.AddValue("adr", "ns3::BaseEndDeviceLorawanMac::ADRBit");
        cmd.AddValue("test", "Use test devices (5s period, 5B payload)", testDev);
        cmd.AddValue("file", "Whether to enable .pcap tracing on gateways", file);
        cmd.AddValue("log", "Whether to enable logs", log);
        cmd.Parse(argc, argv);
        if (auto f = getenv("THE_THINGS_STACK_API_TOKEN_FILE"); f)
        {
            std::ifstream file(f);
            std::getline(file, token);
        }
        NS_ABORT_MSG_IF(token == "...", "Please provide an auth token for The Thing Stack API");
    }

    /* Apply global configurations */
    ///////////////// Real-time operation, necessary to interact with the outside world.
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Config::SetDefault("ns3::BaseEndDeviceLorawanMac::ADRBackoff", BooleanValue(true));
    Config::SetDefault("ns3::BaseEndDeviceLorawanMac::EnableCryptography", BooleanValue(true));
    Config::SetDefault("ns3::BaseEndDeviceLorawanMac::FType",
                       EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));
    ///////////////// Needed to manage the variance introduced by real world interaction
    Config::SetDefault("ns3::ClassAEndDeviceLorawanMac::RecvWinSymb", UintegerValue(16));

    /* Logging options */
    if (log)
    {
        //!> Requirement: build ns3 with debug option
        LogComponentEnable("UdpForwarder", LOG_LEVEL_DEBUG);
        LogComponentEnable("TheThingsStackHelper", LOG_LEVEL_ALL);
        LogComponentEnable("RestApiHelper", LOG_LEVEL_ALL);
        LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_INFO);
        LogComponentEnable("BaseEndDeviceLorawanMac", LOG_LEVEL_INFO);
        /* Monitor state changes of devices */
        LogComponentEnable("EloraTTSExample", LOG_LEVEL_ALL);
        /* Formatting */
        LogComponentEnableAll(LOG_PREFIX_FUNC);
        LogComponentEnableAll(LOG_PREFIX_NODE);
        LogComponentEnableAll(LOG_PREFIX_TIME);
    }

    /*******************
     *  Radio Channel  *
     *******************/

    Ptr<OkumuraHataPropagationLossModel> loss;
    Ptr<NakagamiPropagationLossModel> rayleigh;
    Ptr<LoraChannel> channel;
    {
        // Delay obtained from distance and speed of light in vacuum (constant)
        Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

        // This one is empirical and it encompasses average loss due to distance, shadowing (i.e.
        // obstacles), weather, height
        loss = CreateObject<OkumuraHataPropagationLossModel>();
        loss->SetAttribute("Frequency", DoubleValue(868100000.0));
        loss->SetAttribute("Environment", EnumValue(UrbanEnvironment));
        loss->SetAttribute("CitySize", EnumValue(LargeCity));

        // Here we can add variance to the propagation model with multipath Rayleigh fading
        rayleigh = CreateObject<NakagamiPropagationLossModel>();
        rayleigh->SetAttribute("m0", DoubleValue(1.0));
        rayleigh->SetAttribute("m1", DoubleValue(1.0));
        rayleigh->SetAttribute("m2", DoubleValue(1.0));

        channel = CreateObject<LoraChannel>(loss, delay);
    }

    /*************************
     *  Position & mobility  *
     *************************/

    MobilityHelper mobilityEd;
    MobilityHelper mobilityGw;
    Ptr<RangePositionAllocator> rangeAllocator;
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
        mobilityEd.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        // We define rho to generalize the allocation disk for any number of gateway rings
        double rho = range + 2.0 * gatewayDistance * (gatewayRings - 1);
        rangeAllocator = CreateObject<RangePositionAllocator>();
        rangeAllocator->SetAttribute("rho", DoubleValue(rho));
        rangeAllocator->SetAttribute("ZRV",
                                     StringValue("ns3::UniformRandomVariable[Min=1|Max=10]"));
        rangeAllocator->SetAttribute("range", DoubleValue(range));
        mobilityEd.SetPositionAllocator(rangeAllocator);
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
        rangeAllocator->SetNodes(gateways);

        endDevices.Create(nDevices);
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

    /* Radio side (between end devicees and gateways) */
    LorawanHelper helper;
    NetDeviceContainer gwNetDev;
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
        gwNetDev = helper.Install(phyHelper, macHelper, gateways);

        // Create the LoraNetDevices of the end devices
        phyHelper.SetType("ns3::EndDeviceLoraPhy");
        macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
        helper.Install(phyHelper, macHelper, endDevices);
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
        if (testDev)
        {
            PeriodicSenderHelper appHelper;
            appHelper.SetPeriodGenerator(
                CreateObjectWithAttributes<ConstantRandomVariable>("Constant", DoubleValue(5.0)));
            appHelper.SetPacketSizeGenerator(
                CreateObjectWithAttributes<ConstantRandomVariable>("Constant", DoubleValue(5.0)));
            appHelper.Install(endDevices);
        }
        else
        {
            UrbanTrafficHelper appHelper;
            appHelper.SetDeviceGroups(Commercial);
            appHelper.Install(endDevices);
        }
    }

    /***************************
     *  Simulation and metrics *
     ***************************/

    ///////////////////// Signal handling
    OnInterrupt([](int signal) {
        ttsHelper.CloseConnection(signal);
        OnInterrupt(SIG_DFL); // avoid multiple executions
        exit(0);
    });
    ///////////////////// Register tenant, gateways, and devices on the real server
    ttsHelper.SetApplication(app);
    ttsHelper.InitConnection(apiAddr, apiPort, token);
    ttsHelper.Register(NodeContainer(endDevices, gateways));

    // Initialize SF emulating the ADR algorithm, then add variance to path loss
    std::vector<int> devPerSF(1, nDevices);
    if (initializeSF)
    {
        devPerSF = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);
    }
    loss->SetNext(rayleigh);

#ifdef NS3_LOG_ENABLE
    // Print current configuration
    PrintConfigSetup(nDevices, range, gatewayRings, devPerSF);
    helper.EnableSimulationTimePrinting(Seconds(3600));
#endif // NS3_LOG_ENABLE

    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Phy/$ns3::EndDeviceLoraPhy/EndDeviceState",
        MakeCallback(&OnStateChange));

    if (file)
    {
        helper.EnablePcap("lora", gwNetDev);
    }

    Simulator::Stop(Hours(1) * periods);

    // Start simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
