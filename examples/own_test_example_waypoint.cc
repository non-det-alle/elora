#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE("MobilityTestRun");

using namespace ns3;

// Function to output the node's position
static void PrintNodePosition(Ptr<const MobilityModel> mobility) {
    NS_LOG_INFO("Current position: " << mobility->GetPosition());

    // Schedule the next position update after a specified time interval
    Simulator::Schedule(Seconds(1.0), &PrintNodePosition, mobility);
}

int main(int argc, char *argv[]) {
    LogComponentEnable("MobilityTestRun", LOG_LEVEL_INFO);

    // Create a Node container
    NodeContainer nodes;
    nodes.Create(1);

    // Create a Mobility model and install it on the node
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::WaypointMobilityModel");
    mobility.Install(nodes);

    Ptr<WaypointMobilityModel> waypointMobility = nodes.Get(0)->GetObject<WaypointMobilityModel>();

    

    // Add waypoints
    Vector waypoint1(0.0, 0.0, 0.0);
    Vector waypoint2(100.0, 0.0, 0.0);
    Vector waypoint3(100.0, 100.0, 0.0);
    Vector waypoint4(0.0, 100.0, 0.0);

    waypointMobility->AddWaypoint(Waypoint(Seconds(0.0), waypoint1));
    waypointMobility->AddWaypoint(Waypoint(Seconds(10.0), waypoint2));
    waypointMobility->AddWaypoint(Waypoint(Seconds(20.0), waypoint3));
    waypointMobility->AddWaypoint(Waypoint(Seconds(40.0), waypoint4));

    // Schedule the first position update after a specified time interval
    Simulator::Schedule(Seconds(1.0), &PrintNodePosition, waypointMobility);

    Simulator::Stop(Seconds(50));
    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
