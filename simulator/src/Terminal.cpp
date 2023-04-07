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

#include "Terminal.hpp"
#include "Log.hpp"

#if defined(_WIN32)
#include <conio.h>
#else
#error Platform console not defined
#endif

#define LOG_TAG "Terminal"

Terminal::Terminal()
{
}

Terminal::~Terminal()
{
}

void Terminal::write(uint8_t val)
{
    Log::trc(LOG_TAG).str("Writing ").hex(val, 2).show();
#if defined(_WIN32)
    _putch(val & 0xFF);
#endif
}

uint8_t Terminal::read()
{
    uint8_t result = 0;
#if defined(_WIN32)
    if (_kbhit())
    {
        result = _getch();
    }
#endif
    return result;
}
