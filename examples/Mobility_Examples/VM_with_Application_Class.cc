#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"


NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;


/*
                    Structure for Bike Data

*/
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

/*
                    Application Class

*/
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

MyApplication::MyApplication() : m_node(0), m_printEvent() {}

MyApplication::~MyApplication() {}

void MyApplication::SetNode(Ptr<Node> node) {
    m_node = node;
}

void MyApplication::PrintNodePosition() {
    if (m_node) {
        Vector position = m_node->GetObject<MobilityModel>()->GetPosition();
        NS_LOG_INFO("Node position: " << position.x << ", " << position.y << ", " << position.z);

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

/*
                    Main

*/

int main (int argc, char *argv[]) {
    LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);
    
    std::string filename = "contrib/lorawan/examples/Mobility_Examples/Data_Set/filtered_data.csv";
    std::vector<BikeData> dataset = readDataset(filename);
    
    uint32_t startTimeUnix = 1577836859;  // GMT: Wednesday, January 1, 2020 12:00:59 AM
    uint32_t stopTimeUnix = 1580515175;   // GMT: Friday, January 31, 2020 11:59:35 PM

    // Convert start and end time values to NS-3 Time objects
    Time startTime = Seconds(startTimeUnix);
    Time endTime = Seconds(stopTimeUnix);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(1);

    // Create mobility model and set waypoints

    // Create a Mobility model and install it on the nodes
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    // Add waypoints for node 1
    Vector waypoint1_1(0.0, 0.0, 0.0);
    Vector waypoint1_2(10.0, 0.0, 0.0);   // Modified position
    Vector waypoint1_3(100.0, 100.0, 0.0);
    Vector waypoint1_4(0.0, 100.0, 0.0);

    
    
    Ptr<WaypointMobilityModel> waypointMobility1 = nodes.Get(0)->GetObject<WaypointMobilityModel>();

    waypointMobility1->AddWaypoint(Waypoint(Seconds(0.0), waypoint1_1));
    waypointMobility1->AddWaypoint(Waypoint(Seconds(10.0), waypoint1_2));
    waypointMobility1->AddWaypoint(Waypoint(Seconds(20.0), waypoint1_3));
    waypointMobility1->AddWaypoint(Waypoint(Seconds(40.0), waypoint1_4));

    Ptr<Node> node = nodes.Get(0);

    // Create an instance of your application
    Ptr<MyApplication> app = CreateObject<MyApplication>();

    // Attach the application to the node
    node->AddApplication(app);
    app->SetNode(node);  // Set the node for the application

    std::cout << "Start Time : " << startTime << std::endl;
    std::cout << "End Time : " << endTime << std::endl;
    std::cout << "Difference : " <<  endTime - startTime << " | approx. 30 days" << std::endl;

    // Configure and schedule events for your application
    app->SetStartTime(Seconds(0.0)); //startTime
    app->SetStopTime(Seconds(41.0)); //endTime

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

  return 0;
}