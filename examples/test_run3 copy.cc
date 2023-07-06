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
    long time_started; // now it is sec
    long time_ended; // now it is sec
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
            bikeData.time_started = std::stol(value);

            std::getline(ss, value, ',');
            bikeData.time_ended = std::stol(value);

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
        Ptr<WaypointMobilityModel> waypointMobility = m_node->GetObject<WaypointMobilityModel>();
        NS_LOG_INFO("NODE ID:  " << m_node->GetId() << " | Node position: " << waypointMobility->GetPosition() << " , Time is : " << Simulator::Now().GetSeconds());
        std :: cout << "NODE ID:  " << m_node->GetId() << " | Node position: " << waypointMobility->GetPosition() << " , Time is : " << Simulator::Now().GetSeconds() << std :: endl;
        ScheduleNextPositionPrint();
    }
}

void MyApplication::ScheduleNextPositionPrint() {
    Ptr<WaypointMobilityModel> waypointMobility = m_node->GetObject<WaypointMobilityModel>();
    if(waypointMobility->WaypointsLeft() == 0 && waypointMobility->GetVelocity() == Vector3D(0, 0, 0)){
        Simulator::Cancel(m_printEvent);
    }
    else {
        m_printEvent = Simulator::Schedule(Seconds(1.0), &MyApplication::PrintNodePosition, this);

    }
}

void MyApplication::StartApplication() {
    NS_LOG_INFO("MyApplication::StartApplication called for node = "  << m_node->GetId());
    std :: cout << "MyApplication::StartApplication called for node = "  << m_node->GetId() << std :: endl;
    ScheduleNextPositionPrint();  // Schedule the first position print
}

void MyApplication::StopApplication() {
    NS_LOG_INFO("MyApplication::StopApplication called for node = " << m_node->GetId());
    std :: cout << "MyApplication::StopApplication called for node = " << m_node->GetId() << std :: endl; 
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

    /************************************************
     *     Saving data into vector and map Section  *
     ************************************************/
    std::string filename = "scratch/output.csv";
    std::vector<BikeData> dataset = readDataset(filename);

    // Map
    std::map<std::string, int> myMap;
    std::map<long, int> node_Map_StartTime;
    std::map<long, int> node_Map_EndTime;

    // Set
    std::set<std::string> uniqueBikeNumbers;


    for (const BikeData& data : dataset) {
        uniqueBikeNumbers.insert(data.bikeNumber);
    }

    int key = 0; // For myMap
    for (const std::string& bikeNumber : uniqueBikeNumbers) {
        myMap[bikeNumber] = key++; // Assigning the key
    }

    
    
    // Create nodes
    NodeContainer nodes;
    nodes.Create(myMap.size());

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    // Mobility Pointer
    Ptr<WaypointMobilityModel> waypointMobility;

    // Node Pointer
    Ptr<Node> node;


    /************************************************
     *     Adding waypoints to nodes section        *
     ************************************************/
    int row = 0; 
    for (const BikeData& bike : dataset) {
        for (const auto& pair : myMap) {
            if (pair.first == bike.bikeNumber) {
                // std :: cout << "row = " << row++ << std :: endl;
                if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
                    std :: cout << "row = " << row++ << " is Skipped" << std :: endl;
                }
                else{
                    node = nodes.Get(pair.second);
                    waypointMobility = nodes.Get(pair.second)->GetObject<WaypointMobilityModel>();
                    // Waypoint 1 - Start Position
                    waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_started), Vector(bike.start_lng, bike.start_lat, 0.0)));
                    std :: cout << "Start WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row << std :: endl; 
                    // Waypoint 2 - End Position                
                    waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_ended), Vector(bike.end_lng, bike.end_lat, 0.0)));
                    std :: cout << "End WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row++ << std :: endl;
                    
                    
                    /***********************************************
                    *      Saving Start time in map section        *
                    ************************************************/
                    if(!node_Map_StartTime.empty()){
                        // Checking if the key exists using the find() function
                        auto it = node_Map_StartTime.find(pair.second);
                        if (it != node_Map_StartTime.end()) {
                            std::cout << "Key " << pair.second << " exists in the mapmap[node_Map_StartTime]" << std::endl;
                        } 
                        else {
                            std::cout << "Key " << pair.second << " is added to map[node_Map_StartTime]" << std::endl;
                            node_Map_StartTime.insert(std::make_pair(pair.second, bike.time_started));
                        }
                    }
                    else{
                        node_Map_StartTime.insert(std::make_pair(pair.second, bike.time_started));
                        std::cout << "First Key " << pair.second << " is added to map[node_Map_StartTime]" << std::endl;
                    }

                    /***********************************************
                    *      Saving End time in map section        *
                    ************************************************/
                    if(!node_Map_EndTime.empty()){
                        // Checking if the key exists using the find() function
                        auto it = node_Map_EndTime.find(pair.second);
                        if (it != node_Map_EndTime.end()) {
                            if (node_Map_EndTime[pair.second] <= bike.time_ended){
                                std::cout << "Key " << pair.second << " is updated in the mapmap[node_Map_EndTime], From = " << node_Map_EndTime[pair.second] << " to " << bike.time_ended << std::endl;
                                node_Map_EndTime[pair.second] = bike.time_ended; 
                            }
                            std::cout << "Key " << pair.second << " exists in the mapmap[node_Map_EndTime]" << std::endl;
                        } 
                        else {
                            std::cout << "Key " << pair.second << " is added to map[node_Map_EndTime]" << std::endl;
                            node_Map_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
                        }
                    }
                    else{
                        node_Map_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
                        std::cout << "First Key " << pair.second << " is added to map[node_Map_EndTime]" << std::endl;
                    }
                    std :: cout << "*****************************************************************" << std :: endl;
                }
            }
        }
    }

    /************************************************
     *     Section Just to verify Values            *
     ************************************************/ 
    std :: cout << "*****************************************************************" << std :: endl;
    std :: cout << "Size of node_Map_StartTime Map is = " << node_Map_StartTime.size() << std :: endl;
    std :: cout << "Value Stored in Start Time of Node 687 is = " << node_Map_StartTime[687] << std :: endl;
    std :: cout << "Size of node_Map_EndTime Map is = " << node_Map_EndTime.size() << std :: endl;
    std :: cout << "Value Stored in Start Time of Node 687 is = " << node_Map_EndTime[687] << std :: endl;
    std :: cout << "Size of myMap is = " << myMap.size() << std :: endl; 
    //auto it = node_Map_EndTime.find(687);
    //std :: cout << "Value Stored in myMap of Node 687 is = " << it/*myMap["W01124"]*/ << std :: endl;  // W00581
    std :: cout << "*****************************************************************" << std :: endl;

    /************************************************
     *     Adding waypoints to nodes section        *
     ************************************************/
    
    // Loop through the nodes
    for (const auto& pair : myMap) {
        //std::cout<<"Number = " << i << std :: endl;
        // Create an instance of your application
        node = nodes.Get(pair.second);
        Ptr<MyApplication> app = CreateObject<MyApplication>();
        node->AddApplication(app);
        app->SetNode(node);

        // Configure and schedule events for your application
        app->SetStartTime(Seconds(node_Map_StartTime[pair.second])); //startTime
        app->SetStopTime(Seconds(node_Map_EndTime[pair.second])); //endTime
    }





    /************************************************
     *  Example Section To Display Node Behaviour   *
     ************************************************/
    // Ptr<WaypointMobilityModel> waypointMobility = nodes.Get(0)->GetObject<WaypointMobilityModel>();
    // waypointMobility->AddWaypoint(Waypoint(Seconds(10), Vector(0.0, 0.0, 0.0)));
    // waypointMobility->AddWaypoint(Waypoint(Seconds(20), Vector(100.0, 0.0, 0.0)));
    // waypointMobility->AddWaypoint(Waypoint(Seconds(30), Vector(100.0, 100.0, 0.0)));
    // Ptr<Node> node = nodes.Get(0);
    // Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);

    /************************************************
     *  Example of 1 node with application class    *
     ************************************************/
    // waypointMobility = nodes.Get(1)->GetObject<WaypointMobilityModel>();
    // waypointMobility->AddWaypoint(Waypoint(Seconds(5), Vector(10.0, 0.0, 0.0)));
    // waypointMobility->AddWaypoint(Waypoint(Seconds(10), Vector(0.0, 10.0, 0.0)));
    // waypointMobility->AddWaypoint(Waypoint(Seconds(15), Vector(10.0, 10.0, 0.0)));
    // node = nodes.Get(1);

    // //Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);

    // //Create an instance of your application
    // node = nodes.Get(1);
    // Ptr<MyApplication> app = CreateObject<MyApplication>();
    // node->AddApplication(app);
    // app->SetNode(node);

    // // Configure and schedule events for your application
    // app->SetStartTime(Seconds(5)); //startTime
    // app->SetStopTime(Seconds(33)); //endTime
 

    
    
    Time startTime = Seconds(0);
    Time endTime = Seconds(1000); //2713539
    std :: cout << "*****************************************************************" << std :: endl;
    std::cout << "\nStart Time : " << startTime << std::endl;
    std::cout << "End Time : " << endTime << std::endl;
    std::cout << "Difference : " <<  endTime - startTime << " | approx. 31.42 days" << std::endl;
    std :: cout << "*****************************************************************" << std :: endl;

    Simulator::Stop(endTime); // Set the overall simulation end time 31.42 days
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

  return 0;
}