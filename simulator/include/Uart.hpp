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

#ifndef UART_HPP_INCLUDED
#define UART_HPP_INCLUDED

#include "SystemBusDevice.hpp"
#include "Terminal.hpp"

#include <deque>

/// @brief Simulated National Semiconductor PC16550D UART.
class UartPC16550D : public SystemBusDevice
{
public:
    /// @brief Constructor.
    /// @param baseAddr base address of UART
    UartPC16550D(const Address &baseAddr, Terminal *term = 0);
    ~UartPC16550D();

    void storeByte(const Address &addr, uint8_t val);
    uint8_t readByte(const Address &addr);
    bool decodeAddress(const Address &in, Address &out);
    void addCycles(int cycles);

private:
    // Base address.
    uint32_t mBase;

    // Receiver buffer and FIFO
    std::deque<uint8_t> mRcvrFifo;

    // Transmit holding register and FIFO
    std::deque<uint8_t> mXmitFifo;

    // Registers
    uint8_t mRBR; ///< Receiver buffer register
    uint8_t mTHR; ///< Transmit holding register
    uint8_t mIER; ///< Interrupt enable register
    uint8_t mIIR; ///< Interrupt identification register
    uint8_t mFCR; ///< FIFO control register
    uint8_t mLCR; ///< Line control register
    uint8_t mMCR; ///< Modem control register
    uint8_t mLSR; ///< Line status register
    uint8_t mMSR; ///< Modem status register
    uint8_t mSCR; ///< Scratch register
    uint8_t mDLL; ///< Divisor latch (LSB)
    uint8_t mDLM; ///< Divisor latch (MSB)

    // Number of clock counts per byte transmitted or received
    uint32_t mClocksPerByte;

    // Number of clock counts until the next character will go out.
    int mClocksUntilSend;

    // Flag indicating the RBR has data.
    bool rbrFull;

    // Terminal (when connected to one).
    Terminal *mTerm;

    // Check if there are interrupts and trigger IRQ if appropriate.
    void checkForInterrupts();
    // Set the rate at which bytes will be sent.
    void setByteRate();
    // Send bytes in the transmit buffer if connected.
    void send();
    // Receive the byte.
    void receive(uint8_t val);
    // Set bits in the MSR register. Bits in the mask are set if set==true,
    // else cleared.
    void setMSR(uint8_t mask, bool set);
};

#endif // UART_HPP_INCLUDED
