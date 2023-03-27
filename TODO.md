# TODO List

Consolidated TODO list of things needed for the PC project.

## Kernel

- Refactor to start with POST
  - Set native mode
  - LoRAM check
  - Set stack pointer
  - Device checks
  - HiRAM check
  - Math ROM check
- Delete unused video chip
- "Hello World" to serial out (assume a terminal)

## Simulator

- Write "logs" to stderr instead of stdout
- Update CMake and C++ versions
- Read interrupt vectors from system bus
- Fix bug in XCE switch to native
- Fix JSR indirect indexed bug
- Create PC16550D UART device
- Create 65C22 VIA device
- Add clock ticks to system bus devices
