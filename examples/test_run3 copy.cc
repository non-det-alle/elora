#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"

#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"


NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;

    /***************************
     *      Bike Struture      *
     ***************************/
struct BikeData {
    std::string bikeNumber;
    int duration;
    long started_at_unix;
    long ended_at_unix;
    int start_station;
    int end_station;
    double start_lat;
    double start_lng;
    double end_lat;
    double end_lng; 
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
            bikeData.bikeNumber = value;

            std::getline(ss, value, ',');
            bikeData.duration = std::stoi(value);

            std::getline(ss, value, ',');
            bikeData.started_at_unix = std::stol(value);

            std::getline(ss, value, ',');
            bikeData.ended_at_unix = std::stol(value);

            std::getline(ss, value, ',');
            bikeData.start_station = std::stoi(value);

            std::getline(ss, value, ',');
            bikeData.end_station = std::stoi(value);

            std::getline(ss, value, ',');
            bikeData.start_lat = std::stod(value);

            std::getline(ss, value, ',');
            bikeData.start_lng = std::stod(value);

            std::getline(ss, value, ',');
            bikeData.end_lat = std::stod(value);

            std::getline(ss, value);
            bikeData.end_lng = std::stod(value);

            dataset.push_back(bikeData);
        }

        file.close();
    }
    else {
        std::cerr << "Failed to open the file: " << filename << std::endl;
    }

    return dataset;
}

    /***************************
     *      MY Application     *
     ***************************/
class MyApplication : public Application {
public:
    static TypeId GetTypeId();

    MyApplication();
    virtual ~MyApplication();

    void SetNode(Ptr<Node> node);  // New function to set the node
    void PrintNodePosition();      // New function to print node position

private:
    virtual void StartApplication();
    virtual void StopApplication();
    void ScheduleNextPositionPrint();  // New function to schedule next position print

    Ptr<Node> m_node;      // Node pointer
    EventId m_printEvent;  // EventId for the periodic printing
};

TypeId MyApplication::GetTypeId() {
    static TypeId tid = TypeId("MyApplication")
        .SetParent<Application>()
        .AddConstructor<MyApplication>();
    return tid;
}

MyApplication::MyApplication() : m_node(nullptr), m_printEvent() {}

MyApplication::~MyApplication() {}

void MyApplication::SetNode(Ptr<Node> node) {
    m_node = node;
}

void MyApplication::PrintNodePosition() {
    if (m_node) {
        //Vector position = m_node->GetObject<MobilityModel>()->GetPosition();
        //NS_LOG_INFO("Node ID: " << m_node->GetId() << ", Position: " << position.x << ", " << position.y << ", " << position.z);
        Ptr<ConstantVelocityMobilityModel> cvmm = m_node->GetObject<ConstantVelocityMobilityModel>();
        NS_LOG_INFO("NODE ID:  " << m_node->GetId() << " | Node position: " << cvmm->GetPosition());
        
        ScheduleNextPositionPrint();
    }
}

void MyApplication::ScheduleNextPositionPrint() {
    m_printEvent = Simulator::Schedule(Seconds(1.0), &MyApplication::PrintNodePosition, this);
}

void MyApplication::StartApplication() {
    NS_LOG_INFO("MyApplication::StartApplication called");
    ScheduleNextPositionPrint();  // Schedule the first position print
}

void MyApplication::StopApplication() {
    NS_LOG_INFO("MyApplication::StopApplication called");
    Simulator::Cancel(m_printEvent);  // Cancel the position print event
}

void PrintNodePosition(Ptr<Node> node)
{
    Ptr<WaypointMobilityModel> waypointMobility = node->GetObject<WaypointMobilityModel>();
    Vector3D position = waypointMobility->GetPosition();
    double time = Simulator::Now().GetSeconds();
    std::cout << "Node ID : " << node->GetId() << ", Node position at time " << time << ": (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Waypoint Left = " << waypointMobility->WaypointsLeft() << std :: endl;
    std::cout << "Velocity = " << waypointMobility->GetVelocity() << std :: endl;
    if(waypointMobility->WaypointsLeft() == 0 && waypointMobility->GetVelocity() == Vector3D(0, 0, 0)){
        Simulator::Stop();
    }
    else {
        Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);

    }
}






    /***************************
     *         MAIN            *
     ***************************/

int main (int argc, char *argv[]) {
    LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);
    
    std::string filename = "scratch/output.csv";
    std::vector<BikeData> dataset = readDataset(filename);

    // Map
    std::map<std::string, int> myMap;
    std::set<std::string> uniqueBikeNumbers;

    for (const BikeData& data : dataset) {
        uniqueBikeNumbers.insert(data.bikeNumber);
    }

    int key = 0;
    for (const std::string& bikeNumber : uniqueBikeNumbers) {
        myMap[bikeNumber] = key++; // Assigning the key without counting
    }

    // Create nodes
    NodeContainer nodes;
    nodes.Create(myMap.size());

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    Ptr<WaypointMobilityModel> waypointMobility = nodes.Get(0)->GetObject<WaypointMobilityModel>();
    waypointMobility->AddWaypoint(Waypoint(Seconds(10), Vector(0.0, 0.0, 0.0)));
    waypointMobility->AddWaypoint(Waypoint(Seconds(20), Vector(100.0, 0.0, 0.0)));
    waypointMobility->AddWaypoint(Waypoint(Seconds(30), Vector(100.0, 100.0, 0.0)));
    Ptr<Node> node = nodes.Get(0);
    Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);

    waypointMobility = nodes.Get(1)->GetObject<WaypointMobilityModel>();
    waypointMobility->AddWaypoint(Waypoint(Seconds(5), Vector(10.0, 0.0, 0.0)));
    waypointMobility->AddWaypoint(Waypoint(Seconds(10), Vector(0.0, 10.0, 0.0)));
    waypointMobility->AddWaypoint(Waypoint(Seconds(15), Vector(10.0, 10.0, 0.0)));
    node = nodes.Get(1);

    Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);

    //bool flag = true;
    // for(const BikeData& bike : dataset){
    //     for(const auto& pair : myMap){
    //         if(pair.first == bike.bikeNumber){
                
    //         }
    //     }
    // }
    
    Time startTime = Seconds(0);
    Time endTime = Seconds(2713539);

    std::cout << "\nStart Time : " << startTime << std::endl;
    std::cout << "End Time : " << endTime << std::endl;
    std::cout << "Difference : " <<  endTime - startTime << " | approx. 31.42 days" << std::endl;

    Simulator::Stop(endTime); // Set the overall simulation end time 31.42 days
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

  return 0;
}