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
#include "ns3/basic-energy-source-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/lorawan-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/range-position-allocator.h"
#include "ns3/urban-traffic-helper.h"

#include "ns3/bike-mobility-helper.h"
#include "ns3/bike-application.h"


using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("BikeMobilityExample");
#include "utilities.cc"

int
main(int argc, char* argv[])
{
    /***************************
     *  Simulation parameters  *
     ***************************/

    int periods = 24; // Hours
    int gatewayRings = 2;
    double range = 2426.85; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
    int nDevices = 100;
    std::string sir = "CROCE";
    bool adrEnabled = false;
    bool initializeSF = false;

    std::string file = "None";
    double printPeriod = 0.5;

    /* Expose parameters to command line */
    {
        CommandLine cmd(__FILE__);
        cmd.AddValue("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
        cmd.AddValue("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
        cmd.AddValue("range", "Radius of the device allocation disk around a gateway)", range);
        cmd.AddValue("devices", "Number of end devices to include in the simulation", nDevices);
        cmd.AddValue("sir", "Signal to Interference Ratio matrix used for interference", sir);
        cmd.AddValue("initSF", "Whether to initialize the SFs", initializeSF);
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
        NS_ASSERT((periods >= 0) and (gatewayRings > 0) and (nDevices >= 0));
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
        LogComponentEnable("BaseEndDeviceLorawanMac", LOG_LEVEL_WARN);
        LogComponentEnableAll(LOG_PREFIX_FUNC);
        LogComponentEnableAll(LOG_PREFIX_NODE);
        LogComponentEnableAll(LOG_PREFIX_TIME);
        LogComponentEnable("BikeHelper", LOG_LEVEL_DEBUG);
        LogComponentEnable("BikeApplication", LOG_LEVEL_DEBUG);

    }

    /******************
     *  Radio Channel *
     ******************/

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

    /**************
     *  Mobility  *
     **************/

    MobilityHelper mobilityEd, mobilityGw;

    BikeHelper bikehelper;
    std::string filename = "/etudiants/siscol/k/kayan_mo/elora/ns-3-dev/contrib/lorawan/examples/Mobility_Examples/Data_Set/202003-ns3-biketrips.csv";
    bikehelper.SetFileName(filename);
    int num_end_devices = bikehelper.Get_Num_of_Nodes();

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

        //// End Device mobility
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

    Ptr<Node> server;
    NodeContainer gateways;
    NodeContainer endDevices;
    {
        server = CreateObject<Node>();

        int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
        gateways.Create(nGateways);
        mobilityGw.Install(gateways);
        rangeAllocator->SetNodes(gateways);
        endDevices.Create(num_end_devices);
        bikehelper.Install(endDevices); // Defining the mobility model and waypoints in this function
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
            p2p.Install(server, *gw);

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
        
        
        Ptr<Node> node;
        Ptr<BikeApplication> app; 
        long node_start_time, node_end_time;
        for (const auto& pair : bikehelper.myMap) {
            app = CreateObject<BikeApplication>();
            // Create an instance of your application
            node = endDevices.Get(pair.second);
            // std:: cout << "Node : " << node->GetId() << std::endl; 
            node->AddApplication(app);
            app->SetNode(node);
            // Configure and schedule events for your application
            node_start_time = bikehelper.node_StartTime[pair.second];            
            // std:: cout << "Start Time : " << node_start_time << std::endl; 

            node_end_time = bikehelper.node_EndTime[pair.second];
            // std:: cout << "End Time : " << node_end_time << std::endl; 


            app->SetStartTime(Seconds(node_start_time)); //startTime
            app->SetStopTime(Seconds(node_end_time)); //endTime
        }

        // Install applications in EDs
        // PeriodicSenderHelper appHelper;
        // appHelper.SetPeriodGenerator(
        //     CreateObjectWithAttributes<NormalRandomVariable>("Mean",
        //                                                      DoubleValue(600.0),
        //                                                      "Variance",
        //                                                      DoubleValue(300.0),
        //                                                      "Bound",
        //                                                      DoubleValue(600.0)));
        // appHelper.SetPacketSizeGenerator(
        //     CreateObjectWithAttributes<NormalRandomVariable>("Mean",
        //                                                      DoubleValue(18),
        //                                                      "Variance",
        //                                                      DoubleValue(10),
        //                                                      "Bound",
        //                                                      DoubleValue(18)));
        // ApplicationContainer apps = appHelper.Install(endDevices);
    }

    /***************************
     *  Simulation and metrics *
     ***************************/

    // Initialize SF emulating the ADR algorithm, then add variance to path loss
    std::vector<int> devPerSF(1, nDevices);
    if (initializeSF)
        devPerSF = macHelper.SetSpreadingFactorsUp(endDevices, gateways, channel);
    loss->SetNext(rayleigh);

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
    loraHelper.EnableSimulationTimePrinting(Hours(2));
#else
    // Limit memory usage
    tracker.EnableOldPacketsCleanup(Hours(1));
#endif // NS3_LOG_ENABLE

    // Start simulation
    Simulator::Stop(Hours(periods));
    Simulator::Run();

#ifdef NS3_LOG_ENABLE
    std::cout << tracker.PrintSimulationStatistics(Simulator::Now() - Hours(24));
#endif

    Simulator::Destroy();

    return 0;
}
