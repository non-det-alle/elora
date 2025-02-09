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

#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "network-controller-components.h"
#include "network-status.h"

#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;
class NetworkControllerComponent;

/**
 * This class collects a series of components that deal with various aspects
 * of managing the network, and queries them for action when a new packet is
 * received or other events occur in the network.
 */
class NetworkController : public Object
{
  public:
    static TypeId GetTypeId();

    NetworkController();
    NetworkController(Ptr<NetworkStatus> networkStatus);
    ~NetworkController() override;

    /**
     * Add a new NetworkControllerComponent
     */
    void Install(Ptr<NetworkControllerComponent> component);

    /**
     * Method that is called by the NetworkServer when a new packet is received.
     *
     * \param packet The newly received packet.
     */
    void OnNewPacket(Ptr<const Packet> packet);

    /**
     * Method that is called by the NetworkScheduler just before sending a reply
     * to a certain End Device.
     */
    void BeforeSendingReply(Ptr<EndDeviceStatus> endDeviceStatus);

  protected:
    void DoDispose() override;

  private:
    Ptr<NetworkStatus> m_status;
    std::list<Ptr<NetworkControllerComponent>> m_components;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_CONTROLLER_H */
