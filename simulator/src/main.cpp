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

#include "Interrupt.hpp"
#include "SystemBus.hpp"
#include "Cpu65816.hpp"
#include "Cpu65816Debugger.hpp"

#define LOG_TAG "MAIN"

NativeModeInterrupts nativeInterrupts {
        .coProcessorEnable = 0x0000,
        .brk = 0x0000,
        .abort = 0x0000,
        .nonMaskableInterrupt = 0x0000,
        .reset = 0x0000,
        .interruptRequest = 0x0000,
};

EmulationModeInterrupts emulationInterrupts {
        .coProcessorEnable = 0x0000,
        .unused = 0x0000,
        .abort = 0x0000,
        .nonMaskableInterrupt = 0x0000,
        .reset = 0x0000,
        .brkIrq = 0x0000
};

int main(int argc, char **argv) {
    Log::vrb(LOG_TAG).str("+++ Lib65816 Sample Programs +++").show();

    Rom rom(Address(0x00, 0xE000), "..\\kernel\\dt65pc.rom");

    Ram ram = Ram(0x2);
    ram.storeByte(Address(0x00, 0x0000), 0x18);
    ram.storeByte(Address(0x00, 0x0001), 0xFB);
    ram.storeByte(Address(0x00, 0x0002), 0xA9);
    ram.storeByte(Address(0x00, 0x0003), 0x65);
    ram.storeByte(Address(0x00, 0x0004), 0x12);

    SystemBus systemBus = SystemBus();
    systemBus.registerDevice(&rom);
    systemBus.registerDevice(&ram);

    Cpu65816 cpu(systemBus, &emulationInterrupts, &nativeInterrupts);
    Cpu65816Debugger debugger(cpu);
    debugger.setBreakPoint(Address(0x00, 0x0005));
    debugger.doBeforeStep([]() {});
    debugger.doAfterStep([]() {});

    bool breakPointHit = false;
    debugger.onBreakPoint([&breakPointHit]()  {
        breakPointHit = true;
    });

    while (!breakPointHit) {
        debugger.step();
    }

    debugger.dumpCpu();

    Log::vrb(LOG_TAG).str("+++ Program completed +++").show();
}
