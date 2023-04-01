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

#include "Log.hpp"
#include "Ram.hpp"
#include "Rom.hpp"
#include "Uart.hpp"

#include "Interrupt.hpp"
#include "SystemBus.hpp"
#include "Cpu65816.hpp"
#include "Cpu65816Debugger.hpp"

#define LOG_TAG "MAIN"

int main(int argc, char **argv) {
    Log::vrb(LOG_TAG).str("+++ DT65PC Simulation +++").show();

    Rom kernel(Address(0x00, 0xC000), "..\\kernel\\dt65pc.rom");
    Rom math0(Address(0xE0, 0x0000), "..\\kernel\\rom0.rom");
    Rom math1(Address(0xF0, 0x0000), "..\\kernel\\rom1.rom");
    UartPC16550D uart0(Address(0x00, 0xB000));
    Ram ram = Ram(0x80);

    SystemBus systemBus = SystemBus();
    systemBus.registerDevice(&kernel);
    systemBus.registerDevice(&math0);
    systemBus.registerDevice(&math1);
    systemBus.registerDevice(&uart0);
    systemBus.registerDevice(&ram);

    Cpu65816 cpu(systemBus);
    Cpu65816Debugger debugger(cpu);
    debugger.doBeforeStep([]() {});
    debugger.doAfterStep([]() {});

    bool breakPointHit = false;
    debugger.onBreakPoint([&breakPointHit]()  {
        breakPointHit = true;
    });
    debugger.onStp([&breakPointHit]() {
        breakPointHit = true;
    });

    while (!breakPointHit) {
        debugger.step();
    }

    debugger.dumpCpu();

    Log::vrb(LOG_TAG).str("+++ DT65PC Stopped +++").show();
}
