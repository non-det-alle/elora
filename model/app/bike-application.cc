/*
 * Copyright (c) 2023 CNAM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Moheed Ali Kayani <moheedalikayani@outlook.com>
 *          Alessandro Aimi <alessandro.aimi@orange.com>
 *                          <alessandro.aimi@cnam.fr>
 *
 */

#include "bike-application.h"

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
BikeApplication::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    // Ensure that a WaypointMobilityModel has been installed on the node
    m_mobility = m_node->GetObject<WaypointMobilityModel>();
    NS_ASSERT_MSG(bool(m_mobility) != 0, "No WaypointMobilityModel found on this node");
    LoraApplication::DoInitialize();
}

void
BikeApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_mobility = nullptr;
    LoraApplication::DoDispose();
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
    NS_LOG_INFO("Event Id: " << m_sendEvent.GetUid());
}

void
BikeApplication::SendPacket(void)
{
    NS_LOG_FUNCTION(this);

    NS_LOG_DEBUG("Node position: " << m_mobility->GetPosition());

    uint32_t waypointsLeft = m_mobility->WaypointsLeft();
    if (waypointsLeft == 0 && m_mobility->GetVelocity() == Vector3D(0, 0, 0))
    {
        // No more trips for this bike
        Simulator::Cancel(m_sendEvent);
        return;
    }

    NS_LOG_DEBUG("Waypoints left = " << unsigned(waypointsLeft));
    if (waypointsLeft % 2 != 0)
    {
        Time now = Simulator::Now();
        Time next =
            m_mobility->GetNextWaypoint().time; // next > now is ensured by WaypointMobilityModel
        m_sendEvent = Simulator::Schedule(next - now, &BikeApplication::SendPacket, this);
        return;
    }

    // Create and send a new packet
    Ptr<Packet> packet = Create<Packet>(m_basePktSize);
    NS_LOG_DEBUG("Sending a packet of app payload size " << packet->GetSize());
    m_mac->Send(packet);
    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(m_avgInterval, &BikeApplication::SendPacket, this);
}

} // namespace lorawan
} // namespace ns3
