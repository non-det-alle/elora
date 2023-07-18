#ifndef BIKE_MOBILITY_HELPER_H
#define BIKE_MOBILITY_HELPER_H

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

namespace ns3
{

struct BikeData {
    long time_started; // now it is sec
    long time_ended; // now it is sec
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    std::string bike_id; 
};

class BikeHelper{
    public: 

        BikeHelper();
        ~BikeHelper();

        std::map<std::string, int> myMap;
        std::map<long, int> node_StartTime;
        std::map<long, int> node_EndTime;

        // Function to get the total number of nodes
        int Get_Num_of_Nodes();
        
        // Install Function and add waypoints function
        void Install(NodeContainer nodes) ; 

        // function to set file name
        void SetFileName(const std::string& filename);

    private:

        MobilityHelper mobility;
        std::string filename;
        std::vector<BikeData> dataset;
        Ptr<ns3::WaypointMobilityModel> waypointMobility;
        Ptr<ns3::Node> node;
        
        // Helper function to map end time of node
        std::map<long, int> end_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap);
        
        // Helper function to map start time of node
        std::map<long, int> start_time_of_node(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap);
        
        // function to read data and store it in vector
        std::vector<BikeData> readDataset(const std::string& filename);

        // function to store unique bike id and give them key
        std::map<std::string, int> createBikeNumberMap(const std::vector<BikeData>& dataset);
        
        void SaveWaypointsImpl(const std::vector<BikeData>& dataset, const std::map<std::string, int>& myMap, ns3::NodeContainer nodes);
};



} // namespace ns3
#endif /* BIKE_HELPER_H  */

