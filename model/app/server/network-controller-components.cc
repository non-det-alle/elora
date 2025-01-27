/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "network-controller-components.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("NetworkControllerComponent");

NS_OBJECT_ENSURE_REGISTERED(NetworkControllerComponent);

TypeId
NetworkControllerComponent::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NetworkControllerComponent").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

// Constructor and destructor
NetworkControllerComponent::NetworkControllerComponent()
{
}

NetworkControllerComponent::~NetworkControllerComponent()
{
}

////////////////////////////////
// ConfirmedMessagesComponent //
////////////////////////////////
TypeId
ConfirmedMessagesComponent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ConfirmedMessagesComponent")
                            .SetParent<NetworkControllerComponent>()
                            .AddConstructor<ConfirmedMessagesComponent>()
                            .SetGroupName("lorawan");
    return tid;
}

ConfirmedMessagesComponent::ConfirmedMessagesComponent()
{
    NS_LOG_FUNCTION(this);
}

ConfirmedMessagesComponent::~ConfirmedMessagesComponent()
{
    NS_LOG_FUNCTION(this);
}

void
ConfirmedMessagesComponent::OnReceivedPacket(Ptr<const Packet> packet,
                                             Ptr<EndDeviceStatus> status,
                                             Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << packet << networkStatus);

    // Check whether the received packet requires an acknowledgment.
    Ptr<Packet> myPacket = packet->Copy();
    LorawanMacHeader mHdr;
    myPacket->RemoveHeader(mHdr);
    LoraFrameHeader fHdr;
    fHdr.SetAsUplink();
    myPacket->RemoveHeader(fHdr);

    NS_LOG_INFO("Received packet Mac Header: " << mHdr);
    NS_LOG_INFO("Received packet Frame Header: " << fHdr);

    if (mHdr.GetFType() == LorawanMacHeader::CONFIRMED_DATA_UP)
    {
        NS_LOG_INFO("Packet requires confirmation");

        // Set up the ACK bit on the reply
        status->m_reply.frameHeader.SetAsDownlink();
        status->m_reply.frameHeader.SetAck(true);
        status->m_reply.frameHeader.SetAddress(fHdr.GetAddress());
        status->m_reply.macHeader.SetFType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
        status->m_reply.needsReply = true;

        // Note that the acknowledgment procedure dies here: "Acknowledgments
        // are only snt in response to the latest message received and are never
        // retransmitted". We interpret this to mean that only the current
        // reception window can be used, and that the Ack field should be
        // emptied in case transmission cannot be performed in the current
        // window. Because of this, in this component's OnFailedReply method we
        // void the ack bits.
    }
}

void
ConfirmedMessagesComponent::BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                               Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << status << networkStatus);
    // Nothing to do in this case
}

void
ConfirmedMessagesComponent::OnFailedReply(Ptr<EndDeviceStatus> status,
                                          Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << networkStatus);

    // Empty the Ack bit.
    status->m_reply.frameHeader.SetAck(false);
}

////////////////////////
// LinkCheckComponent //
////////////////////////
TypeId
LinkCheckComponent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LinkCheckComponent")
                            .SetParent<NetworkControllerComponent>()
                            .AddConstructor<LinkCheckComponent>()
                            .SetGroupName("lorawan");
    return tid;
}

LinkCheckComponent::LinkCheckComponent()
{
    NS_LOG_FUNCTION(this);
}

LinkCheckComponent::~LinkCheckComponent()
{
    NS_LOG_FUNCTION(this);
}

void
LinkCheckComponent::OnReceivedPacket(Ptr<const Packet> packet,
                                     Ptr<EndDeviceStatus> status,
                                     Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << packet << networkStatus);

    // We will only act just before reply, when all Gateways will have received
    // the packet.
}

void
LinkCheckComponent::BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                       Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << status << networkStatus);

    Ptr<Packet> myPacket = status->GetLastPacketReceivedFromDevice()->Copy();
    LorawanMacHeader mHdr;
    myPacket->RemoveHeader(mHdr);
    LoraFrameHeader fHdr;
    fHdr.SetAsUplink();
    myPacket->RemoveHeader(fHdr);

    Ptr<LinkCheckReq> command = fHdr.GetMacCommand<LinkCheckReq>();

    // GetMacCommand returns 0 if no command is found
    if (command)
    {
        status->m_reply.needsReply = true;

        // Get the number of gateways that received the packet and the best
        // margin
        uint8_t gwCount = status->GetLastReceivedPacketInfo().gwList.size();

        Ptr<LinkCheckAns> replyCommand = Create<LinkCheckAns>();
        replyCommand->SetGwCnt(gwCount);
        status->m_reply.frameHeader.SetAsDownlink();
        status->m_reply.frameHeader.AddCommand(replyCommand);
        status->m_reply.macHeader.SetFType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    }
    else
    {
        // Do nothing
    }
}

void
LinkCheckComponent::OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << networkStatus);
}
} // namespace lorawan
} // namespace ns3
