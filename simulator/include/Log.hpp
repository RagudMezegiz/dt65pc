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

#ifndef LOG_HPP_INCLUDED
#define LOG_HPP_INCLUDED

#include <cstdint>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

class Log {
    private:
        static Log sDebugLog;
        static Log sVerboseLog;
        static Log sTraceLog;
        static Log sErrorLog;
        static std::ofstream sOut;
        
        Log(const bool);
        const char *mTag;
        const bool mEnabled;
        std::ostringstream mStream;
        
    public:
        /// @brief Set output to a file.
        /// @param fname log file name
        static void out(const std::string& fname);
        
        /// @brief Close the output file, if open.
        static void out();

        /// @brief Return debug log.
        /// @param tag log tag
        static Log& dbg(const char* tag);

        /// @brief Return verbose log.
        /// @param tag log tag
        static Log& vrb(const char* tag);

        /// @brief Return trace log.
        /// @param tag log tag
        static Log& trc(const char* tag);

        /// @brief Return error log.
        /// @param tag log tag
        static Log& err(const char* tag);
        
        /// @brief Write a string.
        /// @param msg string to write
        /// @return this, for chaining
        Log &str(const char* msg);

        /// @brief Write a hex value.
        /// @param val integer value
        /// @return this, for chaining
        Log &hex(uint32_t val);

        /// @brief Write a hex value.
        /// @param val integer value
        /// @param w width in characters
        /// @return this, for chaining
        Log &hex(uint32_t val, uint8_t w);

        /// @brief Write a decimal value.
        /// @param val integer value
        /// @return this, for chaining
        Log &dec(uint32_t val);

        /// @brief Write a decimal value.
        /// @param val integer value
        /// @param w width in characters
        /// @return this, for chaining
        Log &dec(uint32_t val, uint8_t w);

        /// @brief Write a space.
        /// @return this, for chaining
        Log &sp();
        
        /// @brief Write log buffer to the output.
        void show();
};

#endif // LOG_HPP_INCLUDED
