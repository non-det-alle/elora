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

#ifndef BIKE_APPLICATION_H
#define BIKE_APPLICATION_H

#include "ns3/lora-application.h"
#include "ns3/waypoint-mobility-model.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class provides an application that intends to reproduce the behaviour of a LoRa transmitter
 * installed on a bicycle of a bike sharing service of a city. This application exposes two
 * different transmission behaviours (periodical and none) if it is moving or not. It expects a
 * WaypointMobilityModel installed on the node at simulation start-time.
 */
class BikeApplication : public LoraApplication
{
  public:
    BikeApplication();
    virtual ~BikeApplication();

    static TypeId GetTypeId(void);

  protected:
    void DoInitialize() override;
    void DoDispose() override;

  private:
    virtual void StartApplication(void);

    void SendPacket(void);

    Ptr<WaypointMobilityModel> m_mobility;
};

} // namespace lorawan

} // namespace ns3
#endif /* BIKE_APPLICATION_H  */