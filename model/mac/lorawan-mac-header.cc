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

#include "lorawan-mac-header.h"

#include "ns3/log.h"

#include <bitset>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanMacHeader");

LorawanMacHeader::LorawanMacHeader()
    : m_major(0)
{
}

LorawanMacHeader::~LorawanMacHeader()
{
}

TypeId
LorawanMacHeader::GetTypeId()
{
    static TypeId tid =
        TypeId("LorawanMacHeader").SetParent<Header>().AddConstructor<LorawanMacHeader>();
    return tid;
}

TypeId
LorawanMacHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
LorawanMacHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION_NOARGS();

    return 1; // This header only consists in 8 bits
}

void
LorawanMacHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION_NOARGS();

    // The header we need to fill
    uint8_t header = 0;

    // The FType
    header |= m_ftype << 5;

    // Do nothing for the bits that are RFU

    // The major version bits
    header |= m_major;

    // Write the byte
    start.WriteU8(header);

    NS_LOG_DEBUG("Serialization of MAC header: " << std::bitset<8>(header));
}

uint32_t
LorawanMacHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION_NOARGS();

    // Save the byte on a temporary variable
    uint8_t byte;
    byte = start.ReadU8();

    // Get the 2 least significant bits to have the Major
    m_major = byte & 0b11;

    // Move the three most significant bits to the least significant positions
    // to get the FType
    m_ftype = byte >> 5;

    return 1; // the number of bytes consumed.
}

void
LorawanMacHeader::Print(std::ostream& os) const
{
    os << "MessageType=" << unsigned(m_ftype) << std::endl;
    os << "Major=" << unsigned(m_major) << std::endl;
}

void
LorawanMacHeader::SetFType(enum FType ftype)
{
    NS_LOG_FUNCTION(this << ftype);

    m_ftype = ftype;
}

uint8_t
LorawanMacHeader::GetFType() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_ftype;
}

void
LorawanMacHeader::SetMajor(uint8_t major)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT(0 <= major && major < 4);

    m_major = major;
}

uint8_t
LorawanMacHeader::GetMajor() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_major;
}

bool
LorawanMacHeader::IsUplink() const
{
    NS_LOG_FUNCTION_NOARGS();

    return (m_ftype == JOIN_REQUEST) || (m_ftype == UNCONFIRMED_DATA_UP) ||
           (m_ftype == CONFIRMED_DATA_UP);
}

bool
LorawanMacHeader::IsConfirmed() const
{
    NS_LOG_FUNCTION_NOARGS();

    return (m_ftype == CONFIRMED_DATA_DOWN) || (m_ftype == CONFIRMED_DATA_UP);
}
} // namespace lorawan
} // namespace ns3
