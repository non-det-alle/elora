/*
 * Copyright (c) 2023 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "bike-application-helper.h"

#include "ns3/bike-application.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("BikeApplicationHelper");

BikeApplicationHelper::BikeApplicationHelper()
{
    m_factory.SetTypeId("ns3::BikeApplication");
}

BikeApplicationHelper::~BikeApplicationHelper()
{
}

void
BikeApplicationHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
BikeApplicationHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
BikeApplicationHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }
    return apps;
}

Ptr<Application>
BikeApplicationHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);
    Ptr<BikeApplication> app = m_factory.Create<BikeApplication>();
    app->SetNode(node);
    node->AddApplication(app);
    return app;
}

} // namespace lorawan
} // namespace ns3
