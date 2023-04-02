cmake_minimum_required (VERSION 3.1)
project (sim65816)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# All warnings on
set (CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

add_executable(sim65816
    src/Addressing.cpp
    src/Binary.cpp
    src/Cpu65816.cpp
    src/Cpu65816Debugger.cpp
    src/CpuStatus.cpp
    src/Log.cpp
    src/main.cpp
    src/Ram.cpp
    src/Rom.cpp
    src/Stack.cpp
    src/SystemBus.cpp
    src/SystemBusDevice.cpp
    src/Terminal.cpp
    src/Uart.cpp
    src/opcodes/OpCode_ADC.cpp
    src/opcodes/OpCode_AND.cpp
    src/opcodes/OpCode_ASL.cpp
    src/opcodes/OpCode_BIT.cpp
    src/opcodes/OpCode_Branch.cpp
    src/opcodes/OpCode_CMP.cpp
    src/opcodes/OpCode_CPX_CPY.cpp
    src/opcodes/OpCode_EOR.cpp
    src/opcodes/OpCode_INC_DEC.cpp
    src/opcodes/OpCode_Interrupt.cpp
    src/opcodes/OpCode_JumpReturn.cpp
    src/opcodes/OpCode_LDA.cpp
    src/opcodes/OpCode_LDX.cpp
    src/opcodes/OpCode_LDY.cpp
    src/opcodes/OpCode_LSR.cpp
    src/opcodes/OpCode_Misc.cpp
    src/opcodes/OpCode_ORA.cpp
    src/opcodes/OpCode_ROL.cpp
    src/opcodes/OpCode_ROR.cpp
    src/opcodes/OpCode_SBC.cpp
    src/opcodes/OpCode_STA.cpp
    src/opcodes/OpCode_Stack.cpp
    src/opcodes/OpCode_StatusReg.cpp
    src/opcodes/OpCode_STX.cpp
    src/opcodes/OpCode_STY.cpp
    src/opcodes/OpCode_STZ.cpp
    src/opcodes/OpCode_Transfer.cpp
    src/opcodes/OpCode_TSB_TRB.cpp
    src/opcodes/OpCodeTable.cpp
)
target_include_directories(sim65816 PRIVATE ${PROJECT_SOURCE_DIR}/include)
