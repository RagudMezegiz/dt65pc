/*
 * Copyright (c) 2018 Francesco Rigoni.
 * Copyright (C) 2023 David Terhune
 *
 * This file is part of dt65pc.
 * https://github.com/RagudMezegiz/dt65pc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SYSTEM_BUS_HPP_INCLUDED
#define SYSTEM_BUS_HPP_INCLUDED

#include <cstdint>
#include <vector>

#include "SystemBusDevice.hpp"

class SystemBus {
    public:
        void registerDevice(SystemBusDevice* device);
        void storeByte(const Address& address, uint8_t value);
        void storeTwoBytes(const Address& address, uint16_t value);
        uint8_t readByte(const Address& address);
        uint16_t readTwoBytes(const Address& address);
        Address readAddressAt(const Address& address);
        void addCycles(int cycles);

    private:

        std::vector<SystemBusDevice *> mDevices;
};

#endif // SYSTEM_BUS_HPP_INCLUDED
