/*
 * Copyright (c) 2022 Orange SA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef HEX_GRID_POSITION_ALLOCATOR_H
#define HEX_GRID_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"

#include <cmath>

namespace ns3
{

/**
 * This class is an iterable generator for hexagonal grid positions
 */
class HexGridPositionAllocator : public PositionAllocator
{
  public:
    static TypeId GetTypeId();

    HexGridPositionAllocator();
    ~HexGridPositionAllocator() override;

    Vector GetNext() const override;

    int64_t AssignStreams(int64_t stream) override;

    void SetDistance(double distance);

    void SetZ(double z);

  private:
    /**
     * Refer to hexagonal tiling. We build concentric rings
     * of hexagon starting from one hexagon in the center.
     * Hexagons are placed from the center-top of the last
     * ring, in counter-clockwise fashion.
     *
     * We compute the position tracking:
     *  - the index of the ring of hexagons,
     *  - the sector (the 6 sides of the ring
     *    which is itself a hexagon),
     *  - the hexagon index in the ring line
     *    orthogonal to the radius.
     *
     * With this information we build 2 vectors, one radial
     * and the other fase-shifted counter-clockwise by 120 deg.
     */
    Vector ObtainCurrentPosition() const;

    void ResetCoordinates();

    /**
     * The distance between two adjacent nodes
     */
    double m_d;

    /**
     * The vertical position of allocated nodes
     */
    double m_z;

    /**
     * The coordinates used to keep track of allocation progress
     */
    mutable int m_ring, m_sector, m_hex;

    const double p3 = M_PI / 3.0;
};

} // namespace ns3

#endif /* HEX_GRID_POSITION_ALLOCATOR_H */
