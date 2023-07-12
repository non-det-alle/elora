
#include "bike-mobility-helper.h"
#include "ns3/mobility-module.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BikeHelper");

BikeHelper::BikeHelper(){
    NS_LOG_FUNCTION(this);
}

BikeHelper::~BikeHelper(){
    NS_LOG_FUNCTION(this);
}

void
BikeHelper::Install(NodeContainer nodes) 
{
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    SaveWaypointsImpl(this->dataset, this->myMap, nodes);
    NS_LOG_DEBUG("WayPoints Added ...");

    this->node_StartTime = start_time_of_node(this->dataset, this->myMap);
    NS_LOG_DEBUG("Node Start Time is Extracted ...");

    this->node_EndTime = end_time_of_node(this->dataset, this->myMap);
    NS_LOG_DEBUG("Node End Time is Extracted ...");
}

void BikeHelper::SaveWaypointsImpl(const std::vector<BikeData>& dataset, const std::map<std::string, int>& myMap, ns3::NodeContainer nodes)
{
    NS_LOG_DEBUG("HI ...");
    int row = 0; 
    for (const BikeData& bike : dataset) {
        for (const auto& pair : myMap) {
            if (pair.first == bike.bikeNumber) {
                if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
                    NS_LOG_INFO("row = " << row << " is Skipped"); 
                    row++;
                }
                else{
                    this->node = nodes.Get(pair.second);
                    this->waypointMobility = this->node->GetObject<WaypointMobilityModel>();
                    // // Waypoint 1 - Start Position
                    this->waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_started), Vector(bike.start_lng, bike.start_lat, 1.0)));
                    NS_LOG_INFO("Start WayPoint for Node : " << this->node->GetId() << ", Is Saved for row = " << row); 
                    // // Waypoint 2 - End Position                
                    this->waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_ended), Vector(bike.end_lng, bike.end_lat, 1.0)));
                    NS_LOG_INFO("End WayPoint for Node : " << this->node->GetId() << ", Is Saved for row = " << row);
                    row++;
                }
            }
        }
    }
}



void
BikeHelper::SetFileName(const std::string& filename){
    this->filename = filename;
}

int 
BikeHelper::Get_Num_of_Nodes(){
    NS_LOG_DEBUG("Hi From Bike Mobility Helper" << this->filename);

    // Calling Reading from data set function
    this->dataset = readDataset(this->filename);
    NS_LOG_DEBUG("File Read Successfully");

    this->myMap = createBikeNumberMap(this->dataset);

    NS_LOG_DEBUG("Size of Map is : " << myMap.size());


    return this->myMap.size();
}

// function to read data and store it in vector
std::vector<BikeData>
BikeHelper::readDataset(const std::string& filename) {
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

// function to store unique bike id and give them key
std::map<std::string, int> 
BikeHelper::createBikeNumberMap(const std::vector<BikeData>& dataset) {
    std::map<std::string, int> myMap;
    std::set<std::string> uniqueBikeNumbers;

    // Collect unique bike numbers
    for (const BikeData& data : dataset) {
        uniqueBikeNumbers.insert(data.bikeNumber);
    }

    // Assign keys to unique bike numbers and store in the map
    int key = 0;
    for (const std::string& bikeNumber : uniqueBikeNumbers) {
        myMap[bikeNumber] = key++;
    }

    return myMap;
}

// Helper function to map start time of node
std::map<long, int> 
BikeHelper::start_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap) {
    int row = 0; 
    for (const BikeData& bike : dataset) {
        for (const auto& pair : myMap) {
            if (pair.first == bike.bikeNumber) {
                if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
                    NS_LOG_INFO("row = " << row << " is Skipped"); 
                    row++;
                }
                else{
                    if(!this->node_StartTime.empty()){
                        // Checking if the key exists using the find() function
                        auto it = this->node_StartTime.find(pair.second);
                        if (it != this->node_StartTime.end()) {
                            NS_LOG_INFO("Key " << pair.second << " exists in the map[node_StartTime]"); 
                        } 
                        else {
                            NS_LOG_INFO("Key " << pair.second << " is added to map[node_StartTime]"); 
                            this->node_StartTime.insert(std::make_pair(pair.second, bike.time_started));
                        }
                    }
                    else{
                        this->node_StartTime.insert(std::make_pair(pair.second, bike.time_started));
                        //std::cout << "First Key " << pair.second << " is added to map[node_StartTime]" << std::endl;
                    }
                    row++;
                }
            }
        }
    }

    return this->node_StartTime;
}

// Helper function to map end time of node
std::map<long, int> 
BikeHelper::end_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap) {
    int row = 0; 
    for (const BikeData& bike : dataset) {
        for (const auto& pair : myMap) {
            if (pair.first == bike.bikeNumber) {
                if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
                    NS_LOG_INFO("row = " << row << " is Skipped"); 
                    row++;
                }
                else{
                    if(!this->node_EndTime.empty()){
                        // Checking if the key exists using the find() function
                        auto it = this->node_EndTime.find(pair.second);
                        if (it != this->node_EndTime.end()) {
                            if (this->node_EndTime[pair.second] <= bike.time_ended){
                                NS_LOG_INFO("Key " << pair.second << " is updated in the map[node_EndTime], From = " << this->node_EndTime[pair.second] << " to " << bike.time_ended);
                                this->node_EndTime[pair.second] = bike.time_ended; 
                            }
                        } 
                        else {
                            NS_LOG_INFO("Key " << pair.second << " is added to map[node_EndTime]");
                            this->node_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
                        }
                    }
                    else{
                        this->node_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
                    }
                    row++;
                }
            }
        }
    }

    return this->node_EndTime;
}


} // namespace ns3



// // function to print position
// void 
// PrintNodePosition(ns3::Ptr<ns3::Node> node) {
//     ns3::Ptr<ns3::WaypointMobilityModel> waypointMobility = node->GetObject<ns3::WaypointMobilityModel>();
//     ns3::Vector3D position = waypointMobility->GetPosition();
//     double time = ns3::Simulator::Now().GetSeconds();
//     std::cout << "Node ID : " << node->GetId() << ", Node position at time " << time << ": (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
//     std::cout << "Waypoint Left = " << waypointMobility->WaypointsLeft() << std::endl;
//     double nWInSec = waypointMobility->GetNextWaypoint().time.GetSeconds();
//     std::cout << "Next Waypoint Time = " << nWInSec << std::endl;
//     if (waypointMobility->WaypointsLeft() == 0 && waypointMobility->GetVelocity() == ns3::Vector3D(0, 0, 0)) {
//         ns3::Simulator::Stop();
//     }
//     else {
//         if (waypointMobility->WaypointsLeft() % 2 != 0) {
//             ns3::Simulator::Schedule(ns3::Seconds(nWInSec - time), &PrintNodePosition, node);
//         }
//         else {
//             ns3::Simulator::Schedule(ns3::Seconds(1.0), &PrintNodePosition, node);
//         }
//     }
// }

