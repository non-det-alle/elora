/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef BUILDING_PENETRATION_LOSS_H
#define BUILDING_PENETRATION_LOSS_H

#include "ns3/mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/vector.h"

namespace ns3
{
class MobilityModel;

namespace lorawan
{

/**
 * A class implementing the TR 45.820 model for building losses
 */
class BuildingPenetrationLoss : public PropagationLossModel
{
  public:
    static TypeId GetTypeId();

    BuildingPenetrationLoss();

    ~BuildingPenetrationLoss() override;

  private:
    /**
     * Perform the computation of the received power according to the current
     * model.
     */
    double DoCalcRxPower(double txPowerDbm,
                         Ptr<MobilityModel> a,
                         Ptr<MobilityModel> b) const override;

    int64_t DoAssignStreams(int64_t stream) override;

    /**
     * Generate a random p value.
     * The distribution of the returned value is as specified in TR 45.820.
     * \returns A value in the 0-3 range.
     */
    int GetPValue() const;

    /**
     * Get a value to compute the wall loss.
     * The distribution of the returned value is as specified in TR 45.820.
     * \returns A value in the 0-2 range.
     */
    int GetWallLossValue() const;

    /**
     * Compute the wall loss associated to this mobility model
     * \param b The mobility model associated to the node whose wall loss we need
     * to compute.
     * \returns The power loss due to external walls.
     */
    double GetWallLoss(Ptr<MobilityModel> b) const;

    /**
     * Get the Tor1 value used in the TR 45.820 standard to account for internal
     * wall loss.
     * \param b The mobility model of the node we want to compute the value for.
     * \returns The tor1 value.
     */
    double GetTor1(Ptr<MobilityModel> b) const;

    Ptr<UniformRandomVariable> m_uniformRV; //!< An uniform RV

    /**
     * A map linking each mobility model to a p value
     */
    mutable std::map<Ptr<MobilityModel>, int> m_pMap;

    /**
     * A map linking each mobility model to a value deciding its external wall
     * loss.
     */
    mutable std::map<Ptr<MobilityModel>, int> m_wallLossMap;
};
} // namespace lorawan
} // namespace ns3
#endif
