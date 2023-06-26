#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"



NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;

struct BikeData {
    int bikeId;
    int duration;
    double startX;
    double startY;
    double endX;
    double endY;
};

std::vector<BikeData> readDataset(const std::string& filename) {
    std::vector<BikeData> dataset;
    std::ifstream file(filename);

    if (file) {
        std::string line;
        std::getline(file, line); // Skip header line

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string value;
            BikeData bikeData;

            std::getline(ss, value, ',');
            bikeData.bikeId = std::stoi(value);

            std::getline(ss, value, ',');
            bikeData.duration = std::stoi(value);

            std::getline(ss, value, ',');
            bikeData.startX = std::stod(value);

            std::getline(ss, value, ',');
            bikeData.startY = std::stod(value);

            std::getline(ss, value, ',');
            bikeData.endX = std::stod(value);

            std::getline(ss, value);
            bikeData.endY = std::stod(value);

            dataset.push_back(bikeData);
        }

        file.close();
    }
    else {
        std::cerr << "Failed to open the file: " << filename << std::endl;
    }

    return dataset;
}

// Function to output the node's position
static void PrintNodePosition(Ptr<const MobilityModel> mobility, uint32_t nodeId) {
    NS_LOG_INFO("Node " << nodeId << " - Current position: " << mobility->GetPosition());

    // Schedule the next position update after a specified time interval
    Simulator::Schedule(Seconds(1.0), &PrintNodePosition, mobility, nodeId);
}

int main(int argc, char* argv[]){

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    std::string filename = "scratch/data_set_test.csv";
    std::vector<BikeData> dataset = readDataset(filename);

    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);

    // Create a Node container
    NodeContainer nodes;
    nodes.Create(2); // Create two nodes

    // Create a Mobility model and install it on the nodes
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    auto prev = 0;
    bool flag = true;
    int count = 0;

    for (const auto&  bike : dataset) {
        if(bike.bikeId != prev){
            Ptr<WaypointMobilityModel> waypointMobility = nodes.Get(count)->GetObject<WaypointMobilityModel>();

            std::cout << "Bike ID = " << bike.bikeId << std::endl;
            for (const auto& nestedBike : dataset) {
                if (nestedBike.bikeId == bike.bikeId) {
                    if (flag){
                        
                        waypointMobility->AddWaypoint(Waypoint(Seconds(0.0), Vector(nestedBike.startX, nestedBike.startY, 0.0)));
                        std::cout << "Duration (flag) = " << nestedBike.duration << std::endl; 
                        flag = false;    
                    }
                    waypointMobility->AddWaypoint(Waypoint(Seconds(nestedBike.duration), Vector(nestedBike.endX, nestedBike.endY, 0.0)));
                    std::cout << "Duration  = " << nestedBike.duration << std::endl; 
                }
            }
            //Simulator::Schedule(Seconds(1.0), &PrintNodePosition, waypointMobility, count);
            flag = true;
            prev = bike.bikeId;
            count++;
        }
    }

    // Install Internet stack on the nodes
    InternetStackHelper internet;
    internet.Install(nodes);

    // Create a point-to-point link between the nodes
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    // Assign IP addresses to the devices
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Create a UDP echo server application on node 2
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(50.0));

    // Create a UDP echo client application on node 1
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(10.0));

    //Simulator::Schedule(Seconds(1.0), &PrintNodePosition, waypointMobility1, 0);  // Pass node index 0
    //Simulator::Schedule(Seconds(1.0), &PrintNodePosition, waypointMobility2, 1);  // Pass node index 1


    Simulator::Stop(Seconds(50));
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}








