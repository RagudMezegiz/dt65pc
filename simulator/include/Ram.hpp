// Copyright (C) 2018 Francesco Rigoni
// Copyright (C) 2023 David Terhune
// 
// This file is part of dt65pc.
// https://github.com/RagudMezegiz/dt65pc
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
#ifndef RAM_HPP
#define RAM_HPP

#include "SystemBusDevice.hpp"

/// @brief RAM chip of arbitrary size.
/// @details
/// Base address is hard-coded to 0, so there can be only one RAM device
/// in the entire system. That means that a shadow RAM exists beneath every
/// other device with an overlapping address, so RAM must be defined last
/// in the machine description to give priority to other memory-mapped devices
/// on read operations.
class Ram : public SystemBusDevice {
public:
    /// @brief Constructor
    /// @param banks Number of 64K banks
    Ram(uint8_t banks);
    ~Ram();

    void storeByte(const Address &, uint8_t);
    uint8_t readByte(const Address &);
    bool decodeAddress(const Address &, Address &);

private:
    // Number of banks.
    uint8_t mBanks;
    
    // Raw bytes.
    uint8_t *mRam;
};

#endif //RAM_HPP
