#ifndef BIKE_MOBILITY_HELPER_H
#define BIKE_MOBILITY_HELPER_H

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>


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

#endif  // BIKE_MOBILITY_HELPER_H
