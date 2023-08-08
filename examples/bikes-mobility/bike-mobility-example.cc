/*
 * This program creates a network with nodes implementing bicycle mobility.
 */

// ns3 imports
#include "ns3/application-container.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/propagation-delay-model.h"

// lorawan imports
#include "ns3/bike-application-helper.h"
#include "ns3/bike-application.h"
#include "ns3/bike-sharing-mobility-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/lora-channel.h"
#include "ns3/lorawan-helper.h"
#include "ns3/network-server-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("BikeMobilityExample");
#include "../utilities.cc"

int
main(int argc, char* argv[])
{
    /***************************
     *  Simulation parameters  *
     ***************************/

    int periods = 24 * 31; // Hours
    int gatewayRings = 2;
    double range = 2426.85; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
    std::string sir = "CROCE";
    bool adrEnabled = false;

    std::string file = "None";
    double printPeriod = 0.5;

    /* Expose parameters to command line */
    {
        CommandLine cmd(__FILE__);
        cmd.AddValue("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
        cmd.AddValue("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
        cmd.AddValue("range", "Radius of the device allocation disk around a gateway)", range);
        cmd.AddValue("sir", "Signal to Interference Ratio matrix used for interference", sir);
        cmd.AddValue("adr", "Whether to enable online ADR", adrEnabled);
        cmd.AddValue("file",
                     "Output the metrics of the simulation in a file. "
                     "Use to set granularity among DEV|SF|GW|NET. "
                     "Multiple can be passed in the form {DEV,...}",
                     file);
        cmd.AddValue(
            "printPeriod",
            "Periodicity (in hours) of the file printing. In optimized mode, old packets get "
            "automatically cleaned up every 1h to reduce memory usage, thus pay attention to the "
            "temporality of packet-counting metrics.",
            printPeriod);
        cmd.Parse(argc, argv);
        NS_ASSERT((periods >= 0) and (gatewayRings > 0));
    }

    /* Apply global configurations */
    {
        Config::SetDefault("ns3::BaseEndDeviceLorawanMac::ADRBit",
                           BooleanValue(adrEnabled)); //!< ADR bit
        Config::SetDefault("ns3::AdrComponent::SNRDeviceMargin",
                           DoubleValue(10 * log10(-1 / log(0.98))));
    }

    /* Logging options */
    {
        //!> Requirement: build ns3 with debug option
        // LogComponentEnable("BaseEndDeviceLorawanMac", LOG_LEVEL_DEBUG);
        // LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_INFO);
        LogComponentEnable("BikeSharingMobilityHelper", LOG_LEVEL_INFO);
        LogComponentEnable("BikeApplication", LOG_LEVEL_DEBUG);
        LogComponentEnable("WaypointMobilityModel", LOG_LEVEL_DEBUG);
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

        // This one is empirical and it encompasses average loss due to distance, shadowing (i.e.
        // obstacles), weather, height
        auto loss = CreateObject<OkumuraHataPropagationLossModel>();
        loss->SetAttribute("Frequency", DoubleValue(868100000.0));
        loss->SetAttribute("Environment", EnumValue(UrbanEnvironment));
        loss->SetAttribute("CitySize", EnumValue(LargeCity));

        // Here we can add variance to the propagation model with lognormal shadowing
        auto rayleigh = CreateObject<NakagamiPropagationLossModel>();
        rayleigh->SetAttribute("m0", DoubleValue(1.0));
        rayleigh->SetAttribute("m1", DoubleValue(1.0));
        rayleigh->SetAttribute("m2", DoubleValue(1.0));
        loss->SetNext(rayleigh);

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
        std::string filename = "/home/alle/repos/ns-3-dev/contrib/lorawan/examples/bikes-mobility/"
                               "202003-ns3-biketrips.csv";
        mobilityEd.Add(filename);
    }

    /******************
     *  Create Nodes  *
     ******************/

    Ptr<Node> server;
    NodeContainer gateways;
    NodeContainer endDevices;
    int nDevices = 2;
    // int nDevices = mobilityEd.GetNBikes();
    {
        server = CreateObject<Node>();

        int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
        gateways.Create(nGateways);
        mobilityGw.Install(gateways);

        endDevices.Create(nDevices);
        mobilityEd.Install(endDevices);
    }

    /************************
     *  Create Net Devices  *
     ************************/

    LorawanHelper loraHelper;
    LorawanMacHelper macHelper;
    NetDeviceContainer edNetDevices;
    {
        // PointToPoint links between gateways and server
        PointToPointHelper p2p;
        p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        p2p.SetChannelAttribute("Delay", StringValue("2ms"));
        for (auto gw = gateways.Begin(); gw != gateways.End(); ++gw)
        {
            p2p.Install(server, *gw);
        }

        /**
         *  LoRa/LoRaWAN layers
         */

        // General LoRa settings
        loraHelper.EnablePacketTracking();

        // Create a LoraDeviceAddressGenerator
        uint8_t nwkId = 54;
        uint32_t nwkAddr = 1864;
        auto addrGen = CreateObject<LoraDeviceAddressGenerator>(nwkId, nwkAddr);

        // Mac layer settings
        macHelper.SetRegion(LorawanMacHelper::EU);
        macHelper.SetAddressGenerator(addrGen);

        // Physiscal layer settings
        LoraPhyHelper phyHelper;
        phyHelper.SetInterference("IsolationMatrix", EnumValue(sirMap.at(sir)));
        phyHelper.SetChannel(channel);

        // Create the LoraNetDevices of the gateways
        phyHelper.SetType("ns3::GatewayLoraPhy");
        macHelper.SetType("ns3::GatewayLorawanMac");
        loraHelper.Install(phyHelper, macHelper, gateways);

        // Create the LoraNetDevices of the end devices
        phyHelper.SetType("ns3::EndDeviceLoraPhy");
        macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
        edNetDevices = loraHelper.Install(phyHelper, macHelper, endDevices);
    }

    /*************************
     *  Create Applications  *
     *************************/

    {
        // Install the NetworkServer application on the network server
        NetworkServerHelper serverHelper;
        serverHelper.SetEndDevices(endDevices); // Register devices (saves mac layer)
        serverHelper.EnableAdr(adrEnabled);
        serverHelper.Install(server);

        // Install the Forwarder application on the gateways
        ForwarderHelper forwarderHelper;
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

    // Initialize SF emulating the ADR algorithm, then add variance to path loss
    std::vector<int> devPerSF(1, nDevices);

    //! Trace simulation metrics
    Time samplePeriod = Hours(printPeriod);
    if (file != "None")
    {
        loraHelper.EnablePrinting(endDevices, gateways, ParseTraceLevels(file), samplePeriod);
    }

    LoraPacketTracker& tracker = loraHelper.GetPacketTracker();
#ifdef NS3_LOG_ENABLE
    // Print current configuration
    PrintConfigSetup(nDevices, range, gatewayRings, devPerSF);
    loraHelper.EnableSimulationTimePrinting(Hours(24));
#else
    // Limit memory usage
    tracker.EnableOldPacketsCleanup(Hours(1));
#endif // NS3_LOG_ENABLE

    // Start simulation
    Simulator::Stop(Hours(periods));
    Simulator::Run();

#ifdef NS3_LOG_ENABLE
    std::cout << tracker.PrintSimulationStatistics();
#endif

    Simulator::Destroy();

    return 0;
}
