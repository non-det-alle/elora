

#include "bike-application.h"
#include "ns3/mobility-module.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("BikeApplication");

NS_OBJECT_ENSURE_REGISTERED(BikeApplication);

TypeId
BikeApplication::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::BikeApplication")
                            .SetParent<Application>()
                            .AddConstructor<BikeApplication>()
                            .SetGroupName("lorawan");
    return tid;
}

BikeApplication::BikeApplication() : m_node(nullptr), m_printEvent() 
{
    NS_LOG_FUNCTION(this);
}

BikeApplication::~BikeApplication()
{
    NS_LOG_FUNCTION(this);
}

void 
BikeApplication::SetNode(Ptr<Node> node) {
    m_node = node;
}

void 
BikeApplication::PrintNodePosition() {
    if (m_node) {
        Ptr<WaypointMobilityModel> waypointMobility = m_node->GetObject<WaypointMobilityModel>();
        NS_LOG_INFO("NODE ID:  " << m_node->GetId() << " | Node position: " << waypointMobility->GetPosition() << " , Time is : " << Simulator::Now().GetSeconds());
        // std :: cout << "NODE ID:  " << m_node->GetId() << " | Node position: " << waypointMobility->GetPosition() << " , Time is : " << Simulator::Now().GetSeconds() << std :: endl;
        // std::cout << "Waypoint Left = " << waypointMobility->WaypointsLeft() << std :: endl;
        // Get the next waypoint time in sec
        // double nextWaypointInSeconds = waypointMobility->GetNextWaypoint().time.GetSeconds();
        // std::cout << "Next Waypoint arrival Time = " << nextWaypointInSeconds << " , Time is : " << Simulator::Now().GetSeconds() << std :: endl;
        std :: cout<<"_____________________________________________________" << std :: endl;

        ScheduleNextPositionPrint();
    }
}

void 
BikeApplication::ScheduleNextPositionPrint() {
    Ptr<WaypointMobilityModel> waypointMobility = m_node->GetObject<WaypointMobilityModel>();
    if(waypointMobility->WaypointsLeft() == 0 && waypointMobility->GetVelocity() == Vector3D(0, 0, 0)){
        Simulator::Cancel(m_printEvent);
    }
    else {
         if(waypointMobility->WaypointsLeft() %2 != 0){
            std::cout << "Waypoint Left = " << waypointMobility->WaypointsLeft() << std :: endl; 
            double time = Simulator::Now().GetSeconds();   
            double nWInSec = waypointMobility->GetNextWaypoint().time.GetSeconds();
            m_printEvent = Simulator::Schedule(Seconds(nWInSec - time), &BikeApplication::PrintNodePosition, this);
        }
        else{
            std::cout << "Waypoint Left = " << waypointMobility->WaypointsLeft() << std :: endl;    
                           //60 * 60 * 24
            m_printEvent = Simulator::Schedule(Seconds(1), &BikeApplication::PrintNodePosition, this);
        }     
    }
}

// void
// BikeApplication::DoInitialize()
// {
//     NS_LOG_FUNCTION(this);
    
//     //LoraApplication::DoInitialize();
// }

// void
// BikeApplication::DoDispose()
// {
//     NS_LOG_FUNCTION(this);
//     m_node = nullptr;
//     //LoraApplication::DoDispose();
// }


void 
BikeApplication::StartApplication() {
    NS_LOG_INFO("BikeApplication::StartApplication called for node = "  << m_node->GetId());
    //std :: cout << "BikeApplication::StartApplication called for node = "  << m_node->GetId() << std :: endl;
    ScheduleNextPositionPrint();  // Schedule the first position print
}

void 
BikeApplication::StopApplication() {
    NS_LOG_INFO("BikeApplication::StopApplication called for node = " << m_node->GetId());
    //std :: cout << "BikeApplication::StopApplication called for node = " << m_node->GetId() << std :: endl; 
    Simulator::Cancel(m_printEvent);  // Cancel the position print event
}

} // namespace lorawan
} // namespace ns3
