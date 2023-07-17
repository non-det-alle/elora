

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
                            .SetParent<LoraApplication>()
                            .AddConstructor<BikeApplication>()
                            .SetGroupName("lorawan");
    return tid;
}

BikeApplication::BikeApplication()
{
    NS_LOG_FUNCTION(this);
}

BikeApplication::~BikeApplication()
{
    NS_LOG_FUNCTION(this);
}

void
BikeApplication::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    NS_LOG_DEBUG("Starting up application with a first event with a " << m_initialDelay.GetSeconds()
                                                                      << " seconds delay");
    m_sendEvent = Simulator::Schedule(m_initialDelay, &BikeApplication::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
BikeApplication::SendPacket(void)
{
    NS_LOG_FUNCTION(this);
    
    auto waypointMobility = m_node->GetObject<WaypointMobilityModel>();
    NS_ASSERT(bool(waypointMobility) != 0);
    NS_LOG_DEBUG("NODE ID:  " << m_node->GetId() << " | Node position: " << waypointMobility->GetPosition() << " , Time is : " << Simulator::Now().GetSeconds());
    if(waypointMobility->WaypointsLeft() == 0 && waypointMobility->GetVelocity() == Vector3D(0, 0, 0)){
        Simulator::Cancel(m_sendEvent);
        return;
    }

    NS_LOG_DEBUG("Waypoint Left = " << waypointMobility->WaypointsLeft());
    if(waypointMobility->WaypointsLeft() %2 != 0){
        Time now = Simulator::Now();   
        Time next = waypointMobility->GetNextWaypoint().time; // next > now is ensured by WaypointMobilityModel
        m_sendEvent = Simulator::Schedule(next - now, &BikeApplication::SendPacket, this);
        return;
    }

    // Create and send a new packet
    Ptr<Packet> packet = Create<Packet>(m_basePktSize);
    m_mac->Send(packet);
    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(m_avgInterval, &BikeApplication::SendPacket, this);
    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

} // namespace lorawan
} // namespace ns3
