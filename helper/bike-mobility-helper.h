#ifndef BIKE_MOBILITY_HELPER_H
#define BIKE_MOBILITY_HELPER_H

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

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

std::vector<BikeData> readDataset(const std::string& filename);

void PrintNodePosition(ns3::Ptr<ns3::Node> node);

std::map<std::string, int> createBikeNumberMap(const std::vector<BikeData>& dataset);

ns3::Ptr<ns3::WaypointMobilityModel> SaveWaypoints(const std::vector<BikeData>& dataset, const std::map<std::string, int> myMap, ns3::NodeContainer nodes);


#endif  // BIKE_MOBILITY_HELPER_H
