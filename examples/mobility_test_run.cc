#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"

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
// MobilityApplication function
void MobilityApplication(const std::vector<BikeData>& dataset) {
    std::cout << "At Time = " << Simulator::Now().GetSeconds() << "s" << std::endl;
    int prev = 0;
    for (const auto& bike : dataset) {
        if (Simulator::Now().GetSeconds() <= bike.duration && bike.bikeId != prev) {
            if (bike.startX == bike.endX && bike.startY == bike.endY) {
                std::cout << "Bike : " << bike.bikeId << " is Parked" << std::endl;
                prev = bike.bikeId;
            } 
            else {
                std::cout << "Bike : " << bike.bikeId << " is Transmitting Signals" << std::endl;
                prev = bike.bikeId;
            }
        }
    }
    Simulator::Schedule(Seconds(1), &MobilityApplication, dataset);
}

// Define a helper function to invoke MobilityApplication with the dataset
void InvokeMobilityApplication(const std::vector<BikeData>& dataset) {
    MobilityApplication(dataset);
}

int main() {
    
    // Start the simulation
    Simulator::Run();

    std::string filename = "scratch/data_set_test.csv";
    std::vector<BikeData> dataset = readDataset(filename);

    // Create a Node container
    NodeContainer nodes;
    nodes.Create(5); // Create two nodes

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

            //std::cout << "Bike ID = " << bike.bikeId << std::endl;
            for (const auto& nestedBike : dataset) {
                if (nestedBike.bikeId == bike.bikeId) {
                    if (flag){
                        
                        waypointMobility->AddWaypoint(Waypoint(Seconds(0.0), Vector(nestedBike.startX, nestedBike.startY, 0.0)));
                     //   std::cout << "Duration (flag) = " << nestedBike.duration << std::endl; 
                        flag = false;    
                    }
                    waypointMobility->AddWaypoint(Waypoint(Seconds(nestedBike.duration), Vector(nestedBike.endX, nestedBike.endY, 0.0)));
                   // std::cout << "Duration  = " << nestedBike.duration << std::endl; 
                }
            }
            flag = true;
            prev = bike.bikeId;
            count++;
        }
    }

    // Schedule the initial display event
    Simulator::Schedule(Seconds(0), &InvokeMobilityApplication, dataset);


    // Stop the simulation after 41 seconds
    Simulator::Stop(Seconds(41));

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
