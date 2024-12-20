/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "poisson-sender.h"

#include "ns3/double.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("PoissonSender");

NS_OBJECT_ENSURE_REGISTERED(PoissonSender);

TypeId
PoissonSender::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PoissonSender")
                            .SetParent<LoraApplication>()
                            .AddConstructor<PoissonSender>()
                            .SetGroupName("lorawan");
    return tid;
}

PoissonSender::PoissonSender()
{
    NS_LOG_FUNCTION(this);
}

PoissonSender::~PoissonSender()
{
    NS_LOG_FUNCTION(this);
}

void
PoissonSender::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    m_interval = CreateObjectWithAttributes<ExponentialRandomVariable>(
        "Mean",
        DoubleValue(m_avgInterval.GetSeconds()));
    LoraApplication::DoInitialize();
}

void
PoissonSender::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_interval = nullptr;
    LoraApplication::DoDispose();
}

void
PoissonSender::StartApplication()
{
    NS_LOG_FUNCTION(this);
    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    NS_LOG_DEBUG("Starting up application with a first event with a " << m_initialDelay.GetSeconds()
                                                                      << " seconds delay");
    m_sendEvent = Simulator::Schedule(m_initialDelay, &PoissonSender::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
PoissonSender::SendPacket()
{
    NS_LOG_FUNCTION(this);

    // Create and send a new packet
    Ptr<Packet> packet;
    packet = Create<Packet>(m_basePktSize);
    m_mac->Send(packet);

    Time interval = Min(Seconds(m_interval->GetValue()), Days(1));

    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(interval, &PoissonSender::SendPacket, this);

    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

} // namespace lorawan
} // namespace ns3
