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

#ifndef SYSBUS_DEVICE_H
#define SYSBUS_DEVICE_H

#include <cstdint>

#define BANK_SIZE_BYTES                0x10000
#define HALF_BANK_SIZE_BYTES            0x8000
#define PAGE_SIZE_BYTES                    256

class Address {
    private:
        uint8_t mBank;
        uint16_t mOffset;

    public:
        static bool offsetsAreOnDifferentPages(uint16_t, uint16_t);
        static Address sumOffsetToAddress(const Address &, uint16_t);
        static Address sumOffsetToAddressNoWrapAround(const Address &, uint16_t);
        static Address sumOffsetToAddressWrapAround(const Address &, uint16_t);

        Address() = default;
        Address(uint8_t bank, uint16_t offset) : mBank(bank), mOffset(offset) {};

        Address newWithOffset(uint16_t);
        Address newWithOffsetNoWrapAround(uint16_t);
        Address newWithOffsetWrapAround(uint16_t);

        void incrementOffsetBy(uint16_t);
        void decrementOffsetBy(uint16_t);

        void getBankAndOffset(uint8_t *bank, uint16_t *offset) {
            *bank = mBank;
            *offset = mOffset;
        }

        uint8_t getBank() const {
            return mBank;
        }

        uint16_t getOffset() const {
            return mOffset;
        }

        uint32_t getAbsolute() const {
            return (uint32_t)mBank << 16 | mOffset;
        }
};

/// @brief Interface for all devices that reside on the system bus.
class SystemBusDevice {
    public:
        virtual ~SystemBusDevice() {}

        /// @brief Store one byte to the device address.
        /// @param addr address as returned from decodeAddress
        /// @param val value to store
        virtual void storeByte(const Address& addr, uint8_t val) = 0;

        /// @brief Read one byte from the device address.
        /// @param addr address as returned from decodeAddress
        /// @return value at that address
        virtual uint8_t readByte(const Address& addr) = 0;

        /// @brief Decode the address into a device address.
        /// @param in absolute address
        /// @param out decoded address
        virtual bool decodeAddress(const Address& in, Address& out) = 0;

        /// @brief Add clock cycles to the device's cycle count.
        /// @param cycles clock cycles
        virtual void addCycles(int cycles) {}
};

#endif // SYSBUS_DEVICE_H
