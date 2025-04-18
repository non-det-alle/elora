/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Matteo Perin <matteo.perin.2@studenti.unipd.2>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef ADR_COMPONENT_H
#define ADR_COMPONENT_H

#include "network-controller-components.h"
#include "network-status.h"

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

////////////////////////////////////////
// LinkAdrRequest commands management //
////////////////////////////////////////

class AdrComponent : public NetworkControllerComponent
{
    enum CombiningMethod
    {
        AVERAGE,
        MAXIMUM,
        MINIMUM,
    };

  public:
    static TypeId GetTypeId();

    // Constructor
    AdrComponent();
    // Destructor
    ~AdrComponent() override;

    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

  private:
    void AdrImplementation(uint8_t* newDataRate, uint8_t* newTxPower, Ptr<EndDeviceStatus> status);

    double GetMinTxFromGateways(EndDeviceStatus::GatewayList gwList);

    double GetMaxTxFromGateways(EndDeviceStatus::GatewayList gwList);

    double GetAverageTxFromGateways(EndDeviceStatus::GatewayList gwList);

    double GetReceivedPower(EndDeviceStatus::GatewayList gwList);

    double GetMinSNR(const EndDeviceStatus::ReceivedPacketList& packetList, int historyRange);

    double GetMaxSNR(const EndDeviceStatus::ReceivedPacketList& packetList, int historyRange);

    double GetAverageSNR(const EndDeviceStatus::ReceivedPacketList& packetList, int historyRange);

    int GetTxPowerIndex(int txPower);

    // TX power from gateways policy
    enum CombiningMethod tpAveraging;

    // Number of previous packets to consider
    int historyRange;

    // Received SNR history policy
    enum CombiningMethod historyAveraging;

    // SF lower limit
    const int max_dataRate = 5;

    // Minimum transmission power (dBm e.r.p) (Europe)
    const int min_transmissionPower = 0;

    // Maximum transmission power (dBm e.r.p) (Europe)
    const int max_transmissionPower = 14;

    // Vector containing the required SNR for the 6 allowed SF levels
    // ranging from 7 to 12 (the SNR values are in dB).
    double treshold[6] = {-20.0, -17.5, -15.0, -12.5, -10.0, -7.5};

    // Regulate power in the ADR algorithm
    bool m_toggleTxPower;

    // Additional SNR margin to decrease SF/TXPower
    double m_deviceMargin;
};
} // namespace lorawan
} // namespace ns3

#endif
