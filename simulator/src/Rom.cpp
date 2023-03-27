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
#include "Rom.hpp"
#include "Log.hpp"

#include <fstream>

#define LOG_TAG "ROM"

Rom::Rom(const Address& baseAddr, const std::string& filename)
        : mBase(baseAddr) {
    std::ifstream infile(filename, std::ios_base::binary);
    std::copy(std::istreambuf_iterator<char>(infile),
              std::istreambuf_iterator<char>(),
              std::back_inserter(mRom));
    Log::dbg(LOG_TAG).str("Initialized ROM from ").str(filename.c_str())
        .str(" with size ").hex(mRom.size(), 6).show();
}

Rom::~Rom() {
}

uint8_t Rom::readByte(const Address& addr) {
    return mRom[addr.getBank() * BANK_SIZE_BYTES + addr.getOffset()];
}

bool Rom::decodeAddress(const Address& in, Address& out) {
    uint32_t addr = in.getAbsolute() - mBase.getAbsolute();
    out = Address((addr >> 16) & 0xFF, addr & 0xFFFF);
    return addr < mRom.size();
}
