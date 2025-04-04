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

#ifndef GATEWAY_STATUS_H
#define GATEWAY_STATUS_H

#include "ns3/address.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/net-device.h"
#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

class GatewayStatus : public Object
{
  public:
    static TypeId GetTypeId();

    GatewayStatus();
    GatewayStatus(Address address, Ptr<NetDevice> netDevice, Ptr<GatewayLorawanMac> gwMac);
    ~GatewayStatus() override;

    /**
     * Get this gateway's P2P link address.
     */
    Address GetAddress();

    /**
     * Set this gateway's P2P link address.
     */
    void SetAddress(Address address);

    /**
     * Get the NetDevice through which it's possible to contact this gateway from the server.
     */
    Ptr<NetDevice> GetNetDevice();

    /**
     * Set the NetDevice through which it's possible to contact this gateway from the server.
     */
    void SetNetDevice(Ptr<NetDevice> netDevice);

    /**
     * Get a pointer to this gateway's MAC instance.
     */
    Ptr<GatewayLorawanMac> GetGatewayMac();

    /**
     * Set a pointer to this gateway's MAC instance.
     */
    // void SetGatewayMac (Ptr<GatewayLorawanMac> gwMac);

    /**
     * Query whether or not this gateway is available for immediate transmission
     * on this frequency.
     *
     * \param frequency The frequency at which the gateway's availability should
     * be queried.
     * \return True if the gateway's available, false otherwise.
     */
    bool IsAvailableForTransmission(double frequency);

    void SetNextTransmissionTime(Time nextTransmissionTime);
    // Time GetNextTransmissionTime (void);

  protected:
    void DoDispose() override;

  private:
    Address m_address; //!< The Address of the P2PNetDevice of this gateway

    Ptr<NetDevice>
        m_netDevice; //!< The NetDevice through which to reach this gateway from the server

    Ptr<GatewayLorawanMac> m_gatewayMac; //!< The Mac layer of the gateway

    Time m_nextTransmissionTime; //!< This gateway's next transmission time
};
} // namespace lorawan

} // namespace ns3
#endif /* DEVICE_STATUS_H */
