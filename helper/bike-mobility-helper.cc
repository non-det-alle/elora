
#include "bike-mobility-helper.h"



namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BikeHelper");

BikeHelper::BikeHelper(){
    NS_LOG_FUNCTION(this);
}

BikeHelper::~BikeHelper(){
    NS_LOG_FUNCTION(this);
}

ns3::NodeContainer
BikeHelper::create_nodes_helper(){
    NS_LOG_DEBUG("Hi From Bike Mobility Helper" << this->filename);

    // Calling Reading from data set function
    this->dataset = readDataset(this->filename);
    NS_LOG_DEBUG("File Read Successfully");

    this->myMap = createBikeNumberMap(this->dataset);
    NS_LOG_DEBUG("Size of Map is : " << Get_Num_of_Nodes());

    this->nodes.Create(Get_Num_of_Nodes());
    NS_LOG_DEBUG("Nodes Created");
    return this->nodes;
}

void
BikeHelper::SetFileName(const std::string& filename){
    this->filename = filename;
}

int 
BikeHelper::Get_Num_of_Nodes(){
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

// function to return the waypoint mobility object
ns3::Ptr<ns3::WaypointMobilityModel> 
BikeHelper::SaveWaypoints(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap, ns3::NodeContainer nodes){
    Ptr<ns3::WaypointMobilityModel> waypointMobility;
    int row = 0; 
    for (const BikeData& bike : dataset) {
        for (const auto& pair : myMap) {
            if (pair.first == bike.bikeNumber) {
                // std :: cout << "row = " << row++ << std :: endl;
                if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
                    std :: cout << "row = " << row << " is Skipped" << std :: endl;
                    row++;
                }
                else{
                    //node = nodes.Get(pair.second);
                    waypointMobility = nodes.Get(pair.second)->GetObject<ns3::WaypointMobilityModel>();
                    // Waypoint 1 - Start Position
                    waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_started), Vector(bike.start_lng, bike.start_lat, 0.0)));
                    //std :: cout << "Start WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row << std :: endl; 
                    // Waypoint 2 - End Position                
                    waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_ended), Vector(bike.end_lng, bike.end_lat, 0.0)));
                    //std :: cout << "End WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row << std :: endl;
                    row++;
                    //std :: cout << "*****************************************************************" << std :: endl;
                }
            }
        }
    }

    return waypointMobility;
}

} // namespace ns3




// // function to read data and store it in vector
// std::vector<BikeData> 
// readDataset(const std::string& filename) {
//     std::vector<BikeData> dataset;
//     std::ifstream file(filename);

//     if (file) {
//         std::string line;
//         std::getline(file, line); // Skip header line

//         while (std::getline(file, line)) {
//             std::stringstream ss(line);
//             std::string value;
//             BikeData bikeData;

//             std::getline(ss, value, ',');
//             bikeData.bikeNumber = value;

//             std::getline(ss, value, ',');
//             bikeData.duration = std::stoi(value);

//             std::getline(ss, value, ',');
//             bikeData.time_started = std::stol(value);

//             std::getline(ss, value, ',');
//             bikeData.time_ended = std::stol(value);

//             std::getline(ss, value, ',');
//             bikeData.start_station = std::stoi(value);

//             std::getline(ss, value, ',');
//             bikeData.end_station = std::stoi(value);

//             std::getline(ss, value, ',');
//             bikeData.start_lat = std::stod(value);

//             std::getline(ss, value, ',');
//             bikeData.start_lng = std::stod(value);

//             std::getline(ss, value, ',');
//             bikeData.end_lat = std::stod(value);

//             std::getline(ss, value);
//             bikeData.end_lng = std::stod(value);

//             dataset.push_back(bikeData);
//         }

//         file.close();
//     }
//     else {
//         std::cerr << "Failed to open the file: " << filename << std::endl;
//     }

//     return dataset;
// }

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

// // function to store unique bike id and give them key
// std::map<std::string, int> 
// createBikeNumberMap(const std::vector<BikeData>& dataset) {
//     std::map<std::string, int> myMap;
//     std::set<std::string> uniqueBikeNumbers;

//     // Collect unique bike numbers
//     for (const BikeData& data : dataset) {
//         uniqueBikeNumbers.insert(data.bikeNumber);
//     }

//     // Assign keys to unique bike numbers and store in the map
//     int key = 0;
//     for (const std::string& bikeNumber : uniqueBikeNumbers) {
//         myMap[bikeNumber] = key++;
//     }

//     return myMap;
// }

// // function to return the waypoint mobility object
// ns3::Ptr<ns3::WaypointMobilityModel> 
// SaveWaypoints(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap, ns3::NodeContainer nodes){
//     Ptr<ns3::WaypointMobilityModel> waypointMobility;

//     int row = 0; 
//     for (const BikeData& bike : dataset) {
//         for (const auto& pair : myMap) {
//             if (pair.first == bike.bikeNumber) {
//                 // std :: cout << "row = " << row++ << std :: endl;
//                 if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
//                     std :: cout << "row = " << row << " is Skipped" << std :: endl;
//                     row++;
//                 }
//                 else{
//                     //node = nodes.Get(pair.second);
//                     waypointMobility = nodes.Get(pair.second)->GetObject<ns3::WaypointMobilityModel>();
//                     // Waypoint 1 - Start Position
//                     waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_started), Vector(bike.start_lng, bike.start_lat, 0.0)));
//                     //std :: cout << "Start WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row << std :: endl; 
//                     // Waypoint 2 - End Position                
//                     waypointMobility->AddWaypoint(Waypoint(Seconds(bike.time_ended), Vector(bike.end_lng, bike.end_lat, 0.0)));
//                     //std :: cout << "End WayPoint for Node : " << node->GetId() << ", Is Saved for row = " << row << std :: endl;
//                     row++;
//                     //std :: cout << "*****************************************************************" << std :: endl;
//                 }
//             }
//         }
//     }

//     return waypointMobility;
// }

// // Helper function to map start time of node
// std::map<long, int> 
// start_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap) {
//     std::map<long, int> node_Map_StartTime;
//     int row = 0; 
//     for (const BikeData& bike : dataset) {
//         for (const auto& pair : myMap) {
//             if (pair.first == bike.bikeNumber) {
//                 // std :: cout << "row = " << row++ << std :: endl;
//                 if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
//                     //std :: cout << "row = " << row << " is Skipped" << std :: endl;
//                     row++;
//                 }
//                 else{                   
//                     row++;                    
//                     /***********************************************
//                     *      Saving Start time in map section        *
//                     ************************************************/
//                     if(!node_Map_StartTime.empty()){
//                         // Checking if the key exists using the find() function
//                         auto it = node_Map_StartTime.find(pair.second);
//                         if (it != node_Map_StartTime.end()) {
//                             //std::cout << "Key " << pair.second << " exists in the mapmap[node_Map_StartTime]" << std::endl;
//                         } 
//                         else {
//                             //std::cout << "Key " << pair.second << " is added to map[node_Map_StartTime]" << std::endl;
//                             node_Map_StartTime.insert(std::make_pair(pair.second, bike.time_started));
//                         }
//                     }
//                     else{
//                         node_Map_StartTime.insert(std::make_pair(pair.second, bike.time_started));
//                         //std::cout << "First Key " << pair.second << " is added to map[node_Map_StartTime]" << std::endl;
//                     }
//                     //std :: cout << "*****************************************************************" << std :: endl;
//                 }
//             }
//         }
//     }

//     return node_Map_StartTime;
// }

// // Helper function to map end time of node
// std::map<long, int> 
// end_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap) {
//     std::map<long, int> node_Map_EndTime;
//     int row = 0; 
//     for (const BikeData& bike : dataset) {
//         for (const auto& pair : myMap) {
//             if (pair.first == bike.bikeNumber) {
//                 // std :: cout << "row = " << row++ << std :: endl;
//                 if (row == 88563 || row == 149263 || row == 149472 || row ==152101){ // faulty rows
//                     //std :: cout << "row = " << row << " is Skipped" << std :: endl;
//                     row++;
//                 }
//                 else{                   
//                     row++;                    
//                     /***********************************************
//                     *      Saving End time in map section          *
//                     ************************************************/
//                     if(!node_Map_EndTime.empty()){
//                         // Checking if the key exists using the find() function
//                         auto it = node_Map_EndTime.find(pair.second);
//                         if (it != node_Map_EndTime.end()) {
//                             if (node_Map_EndTime[pair.second] <= bike.time_ended){
//                                 //std::cout << "Key " << pair.second << " is updated in the mapmap[node_Map_EndTime], From = " << node_Map_EndTime[pair.second] << " to " << bike.time_ended << std::endl;
//                                 node_Map_EndTime[pair.second] = bike.time_ended; 
//                             }
//                             //std::cout << "Key " << pair.second << " exists in the mapmap[node_Map_EndTime]" << std::endl;
//                         } 
//                         else {
//                             //std::cout << "Key " << pair.second << " is added to map[node_Map_EndTime]" << std::endl;
//                             node_Map_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
//                         }
//                     }
//                     else{
//                         node_Map_EndTime.insert(std::make_pair(pair.second, bike.time_ended));
//                         //std::cout << "First Key " << pair.second << " is added to map[node_Map_EndTime]" << std::endl;
//                     }
//                     //std :: cout << "*****************************************************************" << std :: endl;
//                 }
//             }
//         }
//     }
//     return node_Map_EndTime;
// }



