/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "network-server-helper.h"

#include "ns3/adr-component.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/network-controller-components.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("NetworkServerHelper");

NetworkServerHelper::NetworkServerHelper()
    : m_adrEnabled(false)
{
    m_factory.SetTypeId("ns3::NetworkServer");
    SetAdr("ns3::AdrComponent");
}

NetworkServerHelper::~NetworkServerHelper()
{
}

void
NetworkServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
NetworkServerHelper::SetEndDevices(NodeContainer endDevices)
{
    m_endDevices = endDevices;
}

ApplicationContainer
NetworkServerHelper::Install(Ptr<Node> node)
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
NetworkServerHelper::Install(NodeContainer c)
{
    ApplicationContainer apps;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
NetworkServerHelper::InstallPriv(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);

    Ptr<NetworkServer> app = m_factory.Create<NetworkServer>();

    app->SetNode(node);
    node->AddApplication(app);

    for (uint32_t i = 0; i < node->GetNDevices(); i++)
    {
        // Link the NetworkServer app to its NetDevices
        Ptr<NetDevice> currentNetDevice = node->GetDevice(i);
        currentNetDevice->SetReceiveCallback(MakeCallback(&NetworkServer::Receive, app));

        // Register gateways
        Ptr<Channel> channel = currentNetDevice->GetChannel();
        NS_ASSERT_MSG(bool(DynamicCast<PointToPointChannel>(channel)),
                      "Connection with gateways is not PointToPoint");
        for (uint32_t j = 0; j < channel->GetNDevices(); ++j)
        {
            Ptr<Node> gwNode = channel->GetDevice(j)->GetNode();
            // Point to point, so channel only holds 2 devices
            if (gwNode->GetId() != node->GetId())
            {
                // Add the gateway to the NS list
                app->AddGateway(gwNode, currentNetDevice);
                break;
            }
        }
    }

    // Add the end devices
    app->AddNodes(m_endDevices);

    // Add components to the NetworkServer
    InstallComponents(app);

    return app;
}

void
NetworkServerHelper::EnableAdr(bool enableAdr)
{
    NS_LOG_FUNCTION(this << enableAdr);

    m_adrEnabled = enableAdr;
}

void
NetworkServerHelper::SetAdr(std::string type)
{
    NS_LOG_FUNCTION(this << type);

    m_adrSupportFactory = ObjectFactory();
    m_adrSupportFactory.SetTypeId(type);
}

void
NetworkServerHelper::InstallComponents(Ptr<NetworkServer> netServer)
{
    NS_LOG_FUNCTION(this << netServer);

    // Add Confirmed Messages support
    Ptr<ConfirmedMessagesComponent> ackSupport = CreateObject<ConfirmedMessagesComponent>();
    netServer->AddComponent(ackSupport);

    // Add LinkCheck support
    Ptr<LinkCheckComponent> linkCheckSupport = CreateObject<LinkCheckComponent>();
    netServer->AddComponent(linkCheckSupport);

    // Add Adr support
    if (m_adrEnabled)
    {
        netServer->AddComponent(m_adrSupportFactory.Create<NetworkControllerComponent>());
    }
}

} // namespace lorawan
} // namespace ns3
