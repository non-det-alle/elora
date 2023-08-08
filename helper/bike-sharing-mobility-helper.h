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

#ifndef BIKE_SHARING_MOBILITY_HELPER_H
#define BIKE_SHARING_MOBILITY_HELPER_H

#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"

#include <unordered_map>

namespace ns3
{

struct TripData_t
{
    Time startTime;
    Time endTime;
    Vector startPos;
    Vector endPos;
};

using BikeId_t = std::string;

/**
 * This class is an helper to simulate bicycles in a bike sharing service. It loads datasets
 * containing a list of bycicle trips between locations. Then, it uses the data to install on nodes
 * a waypoint-based mobility that follows the path indicated for a bike in the dataset. Nodes follow
 * a straight line between stations. The input dataset must be a .csv file with the following
 * header line:
 *
 *    'started_at,ended_at,start_x,start_y,end_x,end_y,bike_id'
 *
 * Each subsequent line represents one trip. Time (started_at,ended_at) is in seconds and positions
 * (start/end_x/y) are in meters from the center of the ns-3 the catesian plane. bike_id is any
 * string that identifies a bike. Trips can also be added manually one at a time.
 */

class BikeSharingMobilityHelper
{
    using TripList_t = std::vector<TripData_t>;
    using BikeDataMap_t = std::unordered_map<BikeId_t, TripList_t>;

  public:
    BikeSharingMobilityHelper();
    ~BikeSharingMobilityHelper();

    /**
     * \brief Add a trip to the list of trips in the class.
     *
     * It is up to you to provide new trips in chronological order and that do not overlap in
     * time for the same bike.
     *
     * \param [in] trip the trip to be added to the list of trips of a bike.
     * \param [in] bike the identifier of the bike that will execute the trip.
     */
    void Add(TripData_t trip, BikeId_t bike);

    /**
     * \brief Add the bike trips listed in a file.
     * The file should be a simple text file, with one bike trip per line.
     * It must contain start time, end time, starting X position, starting Y position, ending X
     * position, ending Y position, and a bike ID. Timestamps are in seconds, X/Y coordinates are in
     * meters, and the ID is any string.  The delimiter can be any character, such as ',' or '\\t';
     * the default is a comma ','. The first line will be always skipped (usually has column names).
     *
     * It is up to you to provide trips that are sorted by start time and that do not overlap in
     * time for the same bike.
     *
     * The file is read using CsvReader, which explains how comments
     * and whitespace are handled.
     *
     * \param [in] filePath The path to the input file.
     * \param [in] Z The Z value to use aside X and Y positions.
     * \param [in] delimiter The delimiter character; see CsvReader.
     */
    void Add(const std::string filename, double Z = 1, char delimiter = ',');

    /* Returns the number of distinct bicycles of which we have data available */
    int GetNBikes(void);

    /* Install a waypoint mobility model based on bike data on this node */
    void Install(Ptr<Node> node) const;

    /* Install a bike movement pattern on each node of the container  */
    void Install(NodeContainer container) const;

  private:
    const TripList_t& GetNextBike(void) const;

    BikeDataMap_t m_data;                            //! map of bikes and associated list of trips
    mutable BikeDataMap_t::const_iterator m_current; //!< vector iterator
};

} // namespace ns3
#endif /* BIKE_HELPER_H  */
