#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"


NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;

struct Bike {
    int number;
    int duration; // in minutes
    std::string startDate;
    std::string endDate;
    int startStationNumber;
    int endStationNumber;
    double startLat;
    double startLng;
    double endLat;
    double endLng;
};

void PrintPosition(Ptr<Node> node, int intervals) {
    Ptr<ConstantVelocityMobilityModel> cvmm = node->GetObject<ConstantVelocityMobilityModel>();
    NS_LOG_INFO("Interval = " << intervals << " | Node position: " << cvmm->GetPosition());

    intervals++;
    if (intervals < 10) {
        Simulator::Schedule(Seconds(360), &PrintPosition, node, intervals);
    }
}


std::vector<Bike> loadBikeDataFromFile(const std::string& filename) {
    std::vector<Bike> bikes;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << filename << std::endl;
        return bikes;
    }

    std::string line;
    std::getline(file, line); // Skip the header line

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string item;

        Bike bike;
        std::getline(ss, item, ',');
        bike.number = std::stoi(item); // stoi : to convert string to int
        std::getline(ss, item, ',');
        bike.duration = std::stoi(item);
        std::getline(ss, bike.startDate, ',');
        std::getline(ss, bike.endDate, ',');
        std::getline(ss, item, ',');
        bike.startStationNumber = std::stoi(item);
        std::getline(ss, item, ',');
        bike.endStationNumber = std::stoi(item);
        std::getline(ss, item, ',');
        bike.startLat = std::stod(item); // stod : to covert string to double 
        std::getline(ss, item, ',');
        bike.startLng = std::stod(item);
        std::getline(ss, item, ',');
        bike.endLat = std::stod(item);
        std::getline(ss, item);
        bike.endLng = std::stod(item);

        bikes.push_back(bike);
    }

    file.close();

    return bikes;
}

int main(int argc, char *argv[]) {
    LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);

    //loading data from file
    std::string filename = "scratch/test_data.csv";
    std::vector<Bike> bikes = loadBikeDataFromFile(filename);

    // Create a Node container
    NodeContainer nodes;
    nodes.Create(1);

    // Create a Mobility model and install it on the node
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    // Set start and end positions
    double startLat = bikes[0].startLat;  // Start latitude
    double startLng = bikes[0].startLng; // Start longitude
    double endLat = bikes[0].endLat;    // End latitude
    double endLng = bikes[0].endLng;  // End longitude

    // Calculate velocity vector
    std :: cout << "Duration = " << (bikes[0].duration * 60) << " sec" << std :: endl;   
    double time = (bikes[0].duration * 60); // 600 minutes converted to seconds
    //double interval = time / 2.0;
    Vector velocity((endLng - startLng) / time, (endLat - startLat) / time, 0.0);

    // Set constant velocity for the node
    Ptr<ConstantVelocityMobilityModel> cvmm = nodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
    cvmm->SetVelocity(velocity);

    // Print node position periodically
    int numIntervals = 0;
    Simulator::Schedule(Seconds(360), &PrintPosition, nodes.Get(0), numIntervals);

    // Stop the simulation after the specified duration
    Simulator::Stop(Seconds(time));

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
