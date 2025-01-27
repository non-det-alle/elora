/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef RANGE_POSITION_ALLOCATOR_H
#define RANGE_POSITION_ALLOCATOR_H

#include "ns3/node-container.h"
#include "ns3/position-allocator.h"

#include <cmath>

namespace ns3
{

/**
 * \brief Produce positions in range of a set of nodes.
 */
class RangePositionAllocator : public PositionAllocator
{
  public:
    static TypeId GetTypeId();
    RangePositionAllocator();
    ~RangePositionAllocator() override;

    /**
     * \param rho the value of the radius of the allocation disc
     */
    void SetRho(double rho);

    /**
     * \param range the max range from any gateway
     */
    void SetRange(double range);

    /**
     * \param x  the X coordinate of the center of the disc
     */
    void SetX(double x);

    /**
     * \param y   the Y coordinate of the center of the disc
     */
    void SetY(double y);

    /**
     * \param z   the Z coordinate of all the positions allocated
     */
    void SetZ(double z);

    /**
     * \param z   random variable to extract z coordinates for positions
     */
    void SetZ(Ptr<RandomVariableStream> z);

    /**
     * \param nodes the nodes to be in range of
     */
    void SetNodes(NodeContainer nodes);

    Vector GetNext() const override;
    int64_t AssignStreams(int64_t stream) override;

  private:
    bool OutOfRange(double x, double y, double z) const;

    Ptr<UniformRandomVariable> m_rv; //!< pointer to uniform random variable
    double m_rho;                    //!< value of the radius of the disc
    double m_range;                  //!< the max range from any provided nodes
    double m_x;                      //!< x coordinate of center of disc
    double m_y;                      //!< y coordinate of center of disc
    double m_z;                      //!< z coordinate of the disc
    Ptr<RandomVariableStream> m_zrv; //!< random variable to extract z coordinates
    std::vector<Ptr<Node>> m_nodes;  //!< the nodes to be in range of
};

} // namespace ns3

#endif /* RANGE_POSITION_ALLOCATOR_H */
