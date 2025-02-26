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

#ifndef ONE_SHOT_SENDER_HELPER_H
#define ONE_SHOT_SENDER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/one-shot-sender.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install OneShotSender applications on multiple
 * nodes at once.
 */
class OneShotSenderHelper
{
  public:
    OneShotSenderHelper();

    ~OneShotSenderHelper();

    void SetAttribute(std::string name, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer c) const;

    ApplicationContainer Install(Ptr<Node> node) const;

    void SetSendTime(Time sendTime);

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory;

    Time m_sendTime; //!< Time at which the OneShotSender will be configured to
                     // send the packet
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_SENDER_HELPER_H */
