/*
 * Copyright (c) 2018 Francesco Rigoni.
 * Copyright (C) 2023 David Terhune
 * 
 * This file is part of dt65pc.
 * https://github.com/RagudMezegiz/d565pc
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

#include "Cpu65816.hpp"

#define LOG_TAG "Cpu::executeMisc"

/**
 * This file contains implementations for all OpCodes that didn't fall into other categories.
 */

void Cpu65816::executeMisc(OpCode &opCode) {
    switch (opCode.getCode()) {
        case(0xEB):     // XBA
        {
            uint8_t lowerA = Binary::lower8BitsOf(mA);
            uint8_t higherA = Binary::higher8BitsOf(mA);
            mA = higherA | (((uint16_t)(lowerA)) << 8);
            mCpuStatus.updateSignAndZeroFlagFrom8BitValue(higherA);
            addToProgramAddressAndCycles(1, 3);
            break;
        }
        case(0xDB):     // STP
        {
            addToProgramAddress(1);
            addToCycles(3);

            // What actually happens is that the PHI2 clock is held high,
            // which prevents internal circuitry from doing anything,
            // because things are edge-triggered. There's no "disable PHI2"
            // hidden internal pin that prevents operations, but setting
            // the RES pin high should accomplish essentially the same thing.
            setRESPin(true);
            break;
        }
        case(0xCB):     // WAI
        {
            setRDYPin(false);

            addToProgramAddress(1);
            addToCycles(3);
            break;
        }
        case(0x42):     // WDM
        {
            addToProgramAddress(2);
            addToCycles(2);
            break;
        }
        case(0xEA):     // NOP
        {
            addToProgramAddress(1);
            addToCycles(2);
            break;
        }
        case(0x44):     // MVP
        {
            Address addressOfOpCodeData = getAddressOfOpCodeData(opCode);
            uint8_t destinationBank = mSystemBus.readByte(addressOfOpCodeData);
            addressOfOpCodeData.incrementOffsetBy(1);
            uint8_t sourceBank = mSystemBus.readByte(addressOfOpCodeData);

            Address sourceAddress(sourceBank, mX);
            Address destinationAddress(destinationBank, mY);

            while(mA != 0xFFFF) {
                uint8_t toTransfer = mSystemBus.readByte(sourceAddress);
                mSystemBus.storeByte(destinationAddress, toTransfer);

                sourceAddress.decrementOffsetBy(1);
                destinationAddress.decrementOffsetBy(1);
                mA--;

                addToCycles(7);
            }
            mDB = destinationBank;

            addToProgramAddress(3);
            break;
        }
        case(0x54):     // MVN
        {
            Address addressOfOpCodeData = getAddressOfOpCodeData(opCode);
            uint8_t destinationBank = mSystemBus.readByte(addressOfOpCodeData);
            addressOfOpCodeData.incrementOffsetBy(1);
            uint8_t sourceBank = mSystemBus.readByte(addressOfOpCodeData);

            Address sourceAddress(sourceBank, mX);
            Address destinationAddress(destinationBank, mY);

            while(mA != 0xFFFF) {
                uint8_t toTransfer = mSystemBus.readByte(sourceAddress);
                mSystemBus.storeByte(destinationAddress, toTransfer);

                sourceAddress.incrementOffsetBy(1);
                destinationAddress.incrementOffsetBy(1);
                mA--;

                addToCycles(7);
            }
            mDB = destinationBank;

            addToProgramAddress(3);
            break;
        }
        default:
        {
            LOG_UNEXPECTED_OPCODE(opCode);
        }
    }
}
