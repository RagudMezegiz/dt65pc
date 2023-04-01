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

#include "Log.hpp"

#define HEX_PREFIX "$"

Log Log::sVerboseLog(true);
Log Log::sDebugLog(true);
Log Log::sTraceLog(true);
Log Log::sErrorLog(true);
std::ofstream Log::sOut;

Log::Log(const bool enabled) : mEnabled(enabled) {
}

void Log::out(const std::string& fname) {
    sOut = std::ofstream(fname);
}

void Log::out() {
    if (sOut.is_open()) {
        sOut.close();
    }
}

Log& Log::dbg(const char *tag) {
    sDebugLog.mTag = tag;
    return sDebugLog;
}

Log& Log::vrb(const char *tag) {
    sVerboseLog.mTag = tag;
    return sVerboseLog;
}

Log& Log::trc(const char *tag) {
    sTraceLog.mTag = tag;
    return sTraceLog;
}

Log& Log::err(const char *tag) {
    sErrorLog.mTag = tag;
    return sErrorLog;
}

Log &Log::str(const char *msg) {
    if (mEnabled) mStream << msg;
    return *this;
}

Log &Log::hex(uint32_t val) {
    if (mEnabled) mStream << HEX_PREFIX << std::hex << val;
    return *this;
}

Log &Log::hex(uint32_t val, uint8_t w) {
    if (mEnabled) mStream << HEX_PREFIX << std::setw(w) << std::setfill('0') << std::hex << val;
    return *this;
}

Log &Log::dec(uint32_t val) {
    if (mEnabled) mStream << std::dec << val;
    return *this;
}

Log &Log::dec(uint32_t val, uint8_t w) {
    if (mEnabled) mStream << std::setw(w) << std::setfill('0') << val;
    return *this;
}

Log &Log::sp() {
    if (mEnabled) mStream << " ";
    return *this;
}

void Log::show() {
    if (mEnabled) {
        if (sOut.is_open()) {
            sOut << mTag << ": " << mStream.str() << std::endl;
        } else {
            std::cerr << mTag << ": " << mStream.str() << std::endl;
        }
        mStream.str("");
        mStream.clear();
    }
}

