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

#include "Interrupt.hpp"
#include "Cpu65816.hpp"

#define LOG_TAG "Cpu::executeInterrupt"

/**
 * This file contains the implementation for all OpCodes
 * that deal with interrupts.
 */

void Cpu65816::executeInterrupt(OpCode &opCode) {
    switch (opCode.getCode()) {
        case(0x00):  // BRK
        {
            Address vectorAddress;
            if (mCpuStatus.emulationFlag()) {
                mCpuStatus.setBreakFlag();
                vectorAddress = Address(0x00, EIRQ);
                addToCycles(7);
            } else {
                vectorAddress = Address(0x00, NBRK);
                mStack.push8Bit(mProgramAddress.getBank());
                addToCycles(8);
            }
            mStack.push16Bit(static_cast<uint16_t>(mProgramAddress.getOffset() + 2));
            mStack.push8Bit(mCpuStatus.getRegisterValue());
            mCpuStatus.setInterruptDisableFlag();
            mCpuStatus.clearDecimalFlag();
            setProgramAddress(Address(0x00, mSystemBus.readTwoBytes(vectorAddress)));
            break;
        }
        case(0x02):                 // COP
        {
            if (mCpuStatus.emulationFlag()) {
                mStack.push16Bit(static_cast<uint16_t>(mProgramAddress.getOffset() + 2));
                mStack.push8Bit(mCpuStatus.getRegisterValue());
                mCpuStatus.setInterruptDisableFlag();
                setProgramAddress(Address(0x00, mSystemBus.readTwoBytes(Address(0x00, ECOP))));
                addToCycles(7);
            } else {
                mStack.push8Bit(mProgramAddress.getBank());
                mStack.push16Bit(static_cast<uint16_t>(mProgramAddress.getOffset() + 2));
                mStack.push8Bit(mCpuStatus.getRegisterValue());
                mCpuStatus.setInterruptDisableFlag();
                setProgramAddress(Address(0x00, mSystemBus.readTwoBytes(Address(0x00, NCOP))));
                addToCycles(8);
            }
            mCpuStatus.clearDecimalFlag();
            break;
        }
        case(0x40):                 // RTI
        {
            // Note: The picture in the 65816 programming manual about this looks wrong.
            // This implementation follows the text instead.
            mCpuStatus.setRegisterValue(mStack.pull8Bit());

            if (mCpuStatus.emulationFlag()) {
                Address newProgramAddress(mProgramAddress.getBank(), mStack.pull16Bit());
                mProgramAddress = newProgramAddress;
                addToCycles(6);
            } else {
                uint16_t offset = mStack.pull16Bit();
                uint8_t bank = mStack.pull8Bit();
                Address newProgramAddress(bank, offset);
                mProgramAddress = newProgramAddress;
                addToCycles(7);
            }
            break;
        }
        default: {
            LOG_UNEXPECTED_OPCODE(opCode);
        }
    }
}
