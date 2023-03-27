// Copyright (C) 2023 David Terhune
// 
// This file is part of dt65pc.
// 
// dt65pc is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// dt65pc is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with dt65pc.  If not, see <http://www.gnu.org/licenses/>.
#ifndef ROM_HPP_INCLUDED
#define ROM_HPP_INCLUDED

#include <string>
#include <vector>
#include "SystemBusDevice.hpp"

/// @brief ROM device.
/// @details
/// Base address allows mapping the ROM to any location in memory.
/// Size of ROM is determined by size of file.
class Rom : public SystemBusDevice {
public:
    /// @brief Constructor.
    /// @param baseAddr base address
    /// @param filename file to read ROM data from
    Rom(const Address& baseAddr, const std::string& filename);
    ~Rom();

    void storeByte(const Address &, uint8_t) { /* do nothing */ }
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);

private:
    // Disallow copy construction and assignment.
    Rom(const Rom&);
    Rom& operator=(const Rom&);

    Address mBase;
    std::vector<uint8_t> mRom;
};

#endif // ROM_HPP_INCLUDED
