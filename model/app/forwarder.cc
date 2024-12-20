/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "forwarder.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("Forwarder");

NS_OBJECT_ENSURE_REGISTERED(Forwarder);

TypeId
Forwarder::GetTypeId()
{
    static TypeId tid = TypeId("ns3::Forwarder")
                            .SetParent<Application>()
                            .AddConstructor<Forwarder>()
                            .SetGroupName("lorawan");
    return tid;
}

Forwarder::Forwarder()
{
    NS_LOG_FUNCTION(this);
}

Forwarder::~Forwarder()
{
    NS_LOG_FUNCTION(this);
}

void
Forwarder::SetPointToPointNetDevice(Ptr<PointToPointNetDevice> pointToPointNetDevice)
{
    NS_LOG_FUNCTION(this << pointToPointNetDevice);
    m_pointToPointNetDevice = pointToPointNetDevice;
}

void
Forwarder::SetGatewayLorawanMac(Ptr<GatewayLorawanMac> mac)
{
    NS_LOG_FUNCTION(this << mac);
    m_mac = mac;
}

bool
Forwarder::ReceiveFromLora(Ptr<LorawanMac> mac, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    Ptr<Packet> packetCopy = packet->Copy();
    m_pointToPointNetDevice->Send(packetCopy, m_pointToPointNetDevice->GetBroadcast(), 0x800);
    return true;
}

bool
Forwarder::ReceiveFromPointToPoint(Ptr<NetDevice> pointToPointNetDevice,
                                   Ptr<const Packet> packet,
                                   uint16_t protocol,
                                   const Address& sender)
{
    NS_LOG_FUNCTION(this << packet << protocol << sender);
    Ptr<Packet> packetCopy = packet->Copy();
    m_mac->Send(packetCopy);
    return true;
}

void
Forwarder::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_mac = nullptr;
    m_pointToPointNetDevice = nullptr;
    Application::DoDispose();
}

void
Forwarder::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_mac, "GatewayLorawanMac is not set.");
    NS_ASSERT_MSG(m_pointToPointNetDevice, "PointToPointNetDevice not set.");
}

void
Forwarder::StopApplication()
{
    NS_LOG_FUNCTION_NOARGS();

    // TODO Get rid of callbacks
}

} // namespace lorawan
} // namespace ns3
