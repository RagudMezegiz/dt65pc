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

#ifndef TERMINAL_HPP_INCLUDED
#define TERMINAL_HPP_INCLUDED

#include <cstdint>

/// @brief Serial terminal acting as the remote device connected to the UART.
class Terminal
{
public:
    Terminal();
    ~Terminal();

    /// @brief Write the byte to the terminal.
    /// @param val byte to write
    void write(uint8_t val);

    /// @brief Attempt to read a byte from the terminal.
    /// @return the byte, or 0 if no byte available
    uint8_t read();

private:
    // Disallow copy construction and assignment.
    Terminal(const Terminal &);
    Terminal &operator=(const Terminal &);
};

#endif // TERMINAL_HPP_INCLUDED
