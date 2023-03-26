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
#include "Ram.hpp"

Ram::Ram(uint8_t banks) : mBanks(banks) {
    mRam = new uint8_t[banks * BANK_SIZE_BYTES];
}

Ram::~Ram() {
    delete[] mRam;
}

void Ram::storeByte(const Address &address, uint8_t value) {
    mRam[address.getBank() * BANK_SIZE_BYTES + address.getOffset()] = value;
}

uint8_t Ram::readByte(const Address &address) {
    return mRam[address.getBank() * BANK_SIZE_BYTES + address.getOffset()];
}

bool Ram::decodeAddress(const Address &in, Address &out) {
    out = in;
    return in.getBank() < mBanks;
}
