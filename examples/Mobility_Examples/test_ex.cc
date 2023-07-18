#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

// #include "ns3/bike-application.h"
// #include "ns3/bike-mobility-helper.h"

#include "ns3/bike-mobility-helper.h"
// #include "ns3/bike-application.h"
#include "ns3/mobility-helper.h"


NS_LOG_COMPONENT_DEFINE("WaypointMobility");

using namespace ns3;


int main (int argc, char *argv[]) {

    LogComponentEnable("WaypointMobility", LOG_LEVEL_DEBUG);
    LogComponentEnable("BikeHelper", LOG_LEVEL_DEBUG);

    // MobilityHelper bikehelper;
    // bikehelper.SetMobilityModel("ns3::WaypointMobilityModel");
    // NS_LOG_DEBUG("Mobility Model Selected is = " << bikehelper.GetMobilityModelType());

    BikeHelper bikehelper;
    std::string filename = "/etudiants/siscol/k/kayan_mo/elora/ns-3-dev/contrib/lorawan/examples/Mobility_Examples/Data_Set/202003-ns3-biketrips.csv";
    bikehelper.SetFileName(filename);
    int num_end_devices = bikehelper.Get_Num_of_Nodes();

    
    NodeContainer endDevices;
    endDevices.Create(num_end_devices);
    bikehelper.Install(endDevices);
        


    
    // LogComponentEnable("BikeHelper", LOG_LEVEL_DEBUG);
    // LogComponentEnable("BikeApplication", LOG_LEVEL_DEBUG);

    // // LogComponentEnable("BikeHelper", LOG_LEVEL_INFO);

    // std::string filename = "/etudiants/siscol/k/kayan_mo/elora/ns-3-dev/contrib/lorawan/examples/Mobility_Examples/Data_Set/DataSet.csv";

    // BikeHelper bikehelper;

    // bikehelper.SetFileName(filename);
  
    // NodeContainer nodes = bikehelper.Create_And_Install_Helper();

    // Ptr<Node> node;
    // //Create an instance of your application
    // Ptr<ns3::lorawan::BikeApplication> app = CreateObject<ns3::lorawan::BikeApplication>();
    // // NS_LOG_DEBUG("Node = " << node->GetId());


    // for (const auto& pair : bikehelper.myMap) {
    //     //std::cout<<"Number = " << i << std :: endl;
    //     // Create an instance of your application
    //     node = nodes.Get(pair.second);
    //     node->AddApplication(app);
    //     app->SetNode(node);
    //     // Configure and schedule events for your application
    //     app->SetStartTime(Seconds(bikehelper.node_StartTime[pair.second])); //startTime
    //     app->SetStopTime(Seconds(bikehelper.node_EndTime[pair.second])); //endTime
    // }
    // // Run the simulation
    // Simulator::Run();
    // Simulator::Destroy();


    // auto save = SaveWaypoints(bikehelper.dataset, bikehelper.myMap ,bikehelper.nodes);

    // MobilityHelper mobilityEd;
    // mobilityEd.SetMobilityModel("ns3::WaypointMobilityModel");
    // std :: cout << "Mobility Model Selected is = " << mobilityEd.GetMobilityModelType() << std :: endl;


    // /************************************************
    //  *     Saving data into vector and map Section  *
    //  ************************************************/
    // std::string filename = "contrib/lorawan/examples/Mobility_Examples/Data_Set/DataSet.csv";

    // std::vector<BikeData> dataset;
    // // Helper function to read the dataset and store it in the vector
    // dataset = readDataset(filename);

    // // Map
    // std::map<std::string, int> myMap;
    // // Helper to get unique bike id and assign them key
    // myMap = createBikeNumberMap(dataset);

    // std::map<long, int> node_Map_StartTime;
    // std::map<long, int> node_Map_EndTime;

    // // Create nodes
    // NodeContainer nodes;
    // nodes.Create(myMap.size());

    // MobilityHelper mobility;
    // mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    // mobility.Install(nodes);

    // // Mobility Pointer
    // Ptr<WaypointMobilityModel> waypointMobility;

    // // // Helper to get waypoint object with all the waypoints
    // waypointMobility = SaveWaypoints(dataset, myMap, nodes); // uncomment it when you want to store the nodes waypoints from data set
    
    // // // Helper function to map start time of node
    // // node_Map_StartTime = start_time_of_node(dataset, myMap);

    // // // Helper function to map end time of node
    // // node_Map_EndTime = end_time_of_node(dataset, myMap);

    // // Node Pointer
    // Ptr<Node> node;

    // //Create an instance of your application
    // Ptr<ns3::lorawan::BikeApplication> app = CreateObject<ns3::lorawan::BikeApplication>();

    // /************************************************
    //  *  Example of 1 node with application class    *
    //  ************************************************/
    // // waypointMobility = nodes.Get(1)->GetObject<WaypointMobilityModel>();
    // // waypointMobility->AddWaypoint(Waypoint(Seconds(0), Vector(0.0, 0.0, 0.0))); //5   start
    // // waypointMobility->AddWaypoint(Waypoint(Seconds(10), Vector(0.0, 10.0, 0.0))); //4 end

    // // waypointMobility->AddWaypoint(Waypoint(Seconds(20), Vector(0.0, 10.0, 0.0))); //3  start
    // // waypointMobility->AddWaypoint(Waypoint(Seconds(30), Vector(100.0, 10.0, 0.0))); //2 end

    // // waypointMobility->AddWaypoint(Waypoint(Seconds(40), Vector(100.0, 10.0, 0.0))); //1  start
    // // waypointMobility->AddWaypoint(Waypoint(Seconds(50), Vector(100.0, 100.0, 0.0))); //0 end
    // // node = nodes.Get(1);

    // // // Simulator::Schedule(Seconds(1.0), &PrintNodePosition, node);  
    // // node->AddApplication(app);
    // // app->SetNode(node);

    // // // Configure and schedule events for your application
    // // app->SetStartTime(Seconds(0)); //startTime


    // // app->SetStopTime(Seconds(60)); //endTime

    // /************************************************
    //  *     Section Just to verify Values            *
    //  ************************************************/ 
    // // std :: cout << "*****************************************************************" << std :: endl;
    // // std :: cout << "Size of node_Map_StartTime Map is = " << node_Map_StartTime.size() << std :: endl;
    // // std :: cout << "Value Stored in Start Time of Node 687 is = " << node_Map_StartTime[687] << std :: endl;
    // // std :: cout << "Size of node_Map_EndTime Map is = " << node_Map_EndTime.size() << std :: endl;
    // // std :: cout << "Value Stored in Start Time of Node 687 is = " << node_Map_EndTime[687] << std :: endl;
    // // std :: cout << "Size of myMap is = " << myMap.size() << std :: endl; 
    // // //auto it = node_Map_EndTime.find(687);
    // // //std :: cout << "Value Stored in myMap of Node 687 is = " << it/*myMap["W01124"]*/ << std :: endl;  // W00581
    // // std :: cout << "*****************************************************************" << std :: endl;

    

 
    // Time startTime = Seconds(0);
    // Time endTime = Seconds(2713539); //2713539
    // std :: cout << "*****************************************************************" << std :: endl;
    // std::cout << "Start Time : " << startTime << std::endl;
    // std::cout << "End Time : " << endTime << std::endl;
    // std::cout << "Difference : " <<  endTime - startTime << " | approx. 31.42 days" << std::endl;
    // std :: cout << "*****************************************************************" << std :: endl;

    // Simulator::Stop(endTime); // Set the overall simulation end time 31.42 days
    

  return 0;
}