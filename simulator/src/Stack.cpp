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
#include "Stack.hpp"
#include "Log.hpp"

#define LOG_TAG "Stack"

Stack::Stack(SystemBus *systemBus) :
        mSystemBus(systemBus) {
    setEmulation();
}

Stack::Stack(SystemBus *systemBus, uint16_t stackPointer) :
        mSystemBus(systemBus),
        mStackAddress(0x00, stackPointer) {
    Log::trc(LOG_TAG).str("Set to ").hex(stackPointer, 4).show();
}

void Stack::push8Bit(uint8_t value) {
    mSystemBus->storeByte(mStackAddress, value);
    mStackAddress.decrementOffsetBy(sizeof(uint8_t));
}

void Stack::push16Bit(uint16_t value) {
    auto leastSignificant = (uint8_t)((value) & 0xFF);
    auto mostSignificant =  (uint8_t)(((value) & 0xFF00) >> 8);
    push8Bit(mostSignificant);
    push8Bit(leastSignificant);
}

uint8_t Stack::pull8Bit() {
    mStackAddress.incrementOffsetBy(sizeof(uint8_t));
    return mSystemBus->readByte(mStackAddress);
}

uint16_t Stack::pull16Bit() {
    return (uint16_t)(pull8Bit() | (((uint16_t)pull8Bit()) << 8));
}

uint16_t Stack::getStackPointer() {
    return mStackAddress.getOffset();
}

void Stack::setEmulation() {
    mStackAddress = Address(0x00, 0x0100 | (mStackAddress.getOffset() & 0xFF));
}
