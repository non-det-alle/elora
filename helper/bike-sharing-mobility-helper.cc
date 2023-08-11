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

#include "bike-sharing-mobility-helper.h"

#include "ns3/csv-reader.h"
#include "ns3/waypoint-mobility-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BikeSharingMobilityHelper");

BikeSharingMobilityHelper::BikeSharingMobilityHelper()
{
}

BikeSharingMobilityHelper::~BikeSharingMobilityHelper()
{
}

void
BikeSharingMobilityHelper::Add(TripData_t trip, BikeId_t bike)
{
    m_data[bike].push_back(trip);
    m_current = m_data.begin();
}

void
BikeSharingMobilityHelper::Add(const std::string filePath,
                               double Z /* = 1 */,
                               char delimiter /* = ',' */)
{
    NS_LOG_FUNCTION(this << filePath << std::string("'") + delimiter + "'");

    NS_ASSERT(std::ifstream(filePath).good()); // Chek if file can be opened

    CsvReader csv(filePath, delimiter);
    csv.FetchNextRow(); // Skip row with column names
    while (csv.FetchNextRow())
    {
        if (csv.ColumnCount() == 1)
        {
            // comment line
            continue;
        }

        TripData_t td;
        BikeId_t id;

        double tmp;
        bool ok = csv.GetValue(0, tmp);
        NS_LOG_LOGIC("read started_at: " << tmp << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading started_at");
        td.startTime = Seconds(tmp);
        ok = csv.GetValue(1, tmp);
        NS_LOG_LOGIC("read ended_at = " << tmp << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading ended_at");
        td.endTime = Seconds(tmp);

        ok = csv.GetValue(2, td.startPos.x);
        NS_LOG_LOGIC("read start_x = " << td.startPos.x << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading start_x");
        ok = csv.GetValue(3, td.startPos.y);
        NS_LOG_LOGIC("read start_y = " << td.startPos.y << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading start_y");

        ok = csv.GetValue(4, td.endPos.x);
        NS_LOG_LOGIC("read end_x = " << td.endPos.x << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading end_x");
        ok = csv.GetValue(5, td.endPos.y);
        NS_LOG_LOGIC("read end_y = " << td.endPos.y << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading end_y");

        ok = csv.GetValue(6, id);
        NS_LOG_LOGIC("read bike_id = " << id << (ok ? " ok" : " FAIL"));
        NS_ASSERT_MSG(ok, "failed reading bike_id");

        td.startPos.z = Z;
        td.endPos.z = Z;

        Add(td, id);

    } // while FetchNextRow
    NS_LOG_DEBUG("read " << csv.RowNumber() - 1 << " rows");
}

int
BikeSharingMobilityHelper::GetNBikes()
{
    int size = m_data.size();
    NS_LOG_DEBUG("Number of bikes in the dataset: " << size);
    return size;
}

const BikeSharingMobilityHelper::TripList_t&
BikeSharingMobilityHelper::GetNextBike() const
{
    const TripList_t& tl = (*m_current).second;
    NS_LOG_DEBUG("bikeID=" << (*m_current).first);
    m_current++;
    if (m_current == m_data.end())
    {
        NS_LOG_WARN("Already assigned all bikes, restarting from the first.");
        m_current = m_data.begin();
    }
    return tl;
}

void
BikeSharingMobilityHelper::Install(Ptr<Node> node) const
{
    Ptr<Object> object = node;
    auto model = object->GetObject<WaypointMobilityModel>();
    if (bool(model))
    {
        NS_LOG_WARN("Waypoint mobility model already present on node " << node->GetId()
                                                                       << ". Nothing was done.");
        return;
    }

    model = CreateObject<WaypointMobilityModel>();
    NS_LOG_DEBUG("node=" << object << ", mob=" << model);
    object->AggregateObject(model);

    const TripList_t& triplist = GetNextBike();
    for (const auto& t : triplist)
    {
        model->AddWaypoint(Waypoint(t.startTime, t.startPos));
        NS_LOG_INFO("Added trip-start waypoint: " << Waypoint(t.startTime, t.startPos));
        model->AddWaypoint(Waypoint(t.endTime, t.endPos));
        NS_LOG_INFO("Added trip-end waypoint: " << Waypoint(t.endTime, t.endPos));
    }
}

void
BikeSharingMobilityHelper::Install(NodeContainer c) const
{
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Install(*i);
    }
}

} // namespace ns3
