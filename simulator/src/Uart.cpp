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

#include "Uart.hpp"
#include "Log.hpp"

#define LOG_TAG "Uart"

// Number of bytes in FIFOs.
#define FIFO_SIZE 16

// IIR bit flags
#define FIFO_INT 0xC0

// FCR bit flags
#define FIFO_ENABLE 1
#define RCVR_FIFO_RESET 2
#define XMIT_FIFO_RESET 4
#define RCVR_TRG_MASK 0xC0

// LCR bit flags
#define DLAB 0x80

// MCR bit flags
#define DTR 1
#define RTS 2
#define OUT1 4
#define OUT2 8
#define LOOPBACK 0x10

// LSR bit flags
#define DR 1
#define OE 2
#define PE 4
#define FE 8
#define BI 0x10
#define THRE 0x20
#define TEMT 0x40
#define FIFO_ERR 0x80

// MSR bit flags
#define DCTS 1
#define DDSR 2
#define TERI 4
#define DDCD 8
#define CTS 0x10
#define DSR 0x20
#define RI 0x40
#define DCD 0x80

UartPC16550D::UartPC16550D(const Address &baseAddr, Terminal *term) : mBase(baseAddr.getAbsolute()),
                                                                      mIER(0),
                                                                      mIIR(1),
                                                                      mFCR(0),
                                                                      mLCR(0),
                                                                      mMCR(0),
                                                                      mLSR(0x60),
                                                                      mMSR(0),
                                                                      mClocksPerByte(0xFFFFFFFF),
                                                                      rbrFull(false),
                                                                      mTerm(term)
{
}

UartPC16550D::~UartPC16550D()
{
}

void UartPC16550D::storeByte(const Address &addr, uint8_t val)
{
    switch (addr.getOffset())
    {
    case 0:
        if (mLCR & DLAB)
        {
            // Divisor latch access bit set, so write to DLL
            mDLL = val;
            setByteRate();
        }
        else
        {
            // Divisor latch access bit not set, so write to transmit
            // holding register or transmit FIFO.
            if (mFCR & FIFO_ENABLE)
            {
                // Write to transmit FIFO.
                if (mXmitFifo.size() < FIFO_SIZE)
                {
                    Log::trc(LOG_TAG).str("Pushing to FIFO ").hex(val, 2).show();
                    mXmitFifo.push_back(val);
                }
            }
            else
            {
                // FIFO not enabled; write to THR
                Log::trc(LOG_TAG).str("Setting THR ").hex(val, 2).show();
                mTHR = val;
            }
            // Either a FIFO write or a THR write clears both THRE and TEMT
            mLSR &= ~(THRE | TEMT);

            // Now that something is in the transmit buffer, send it
            send();
        }
        break;

    case 1:
        if (mLCR & DLAB)
        {
            // Divisor latch access bit set, so write to DLM.
            mDLM = val;
            setByteRate();
        }
        else
        {
            // Divisor latch access bit not set, so write to IER. Bits 4-7
            // are hardwired to zero, so keep those clear.
            mIER = val & 0xF;
            checkForInterrupts();
        }
        break;

    case 2:
        val &= 0xCF; // clear reserved bits
        if (mFCR & val & FIFO_ENABLE)
        {
            // FIFO enable bit set in both, so the other bits matter.
            if (val & RCVR_FIFO_RESET)
            {
                mRcvrFifo.clear();
                val &= ~RCVR_FIFO_RESET; // bit auto-clears
            }
            if (val & XMIT_FIFO_RESET)
            {
                mXmitFifo.clear();
                val &= ~XMIT_FIFO_RESET; // bit auto-clears
            }
        }
        else if (val & FIFO_ENABLE)
        {
            // Setting FIFO mode. Clear FIFOs.
            mRcvrFifo.clear();
            mXmitFifo.clear();
            rbrFull = false;
            mFCR = FIFO_ENABLE; // clear all bits except FIFO_ENABLE
            mIIR |= FIFO_INT;   // set FIFO interrupt bits in IIR
        }
        else
        {
            // val does not have FIFO_ENABLE bit.
            if (mFCR & FIFO_ENABLE)
            {
                // Unsetting FIFO mode - clear FIFOs
                mRcvrFifo.clear();
                mXmitFifo.clear();
            }
            mFCR = 0;          // clear FCR
            mIIR &= ~FIFO_INT; // clear FIFO interrupt bits in IIR
        }
        break;

    case 3:
        mLCR = val;
        break;

    case 4:
        mMCR = val & 0x1F; // top 3 bits always zero
        if (mMCR & LOOPBACK)
        {
            // Set bits in MSR that correspond to MCR in loopback mode.
            setMSR(CTS, mMCR & RTS);
            setMSR(DSR, mMCR & DTR);
            setMSR(RI, mMCR & OUT1);
            setMSR(DCD, mMCR & OUT2);
            checkForInterrupts();
        }
        break;

    case 5:
    case 6:
        // LSR and MSR aren't writable
        break;

    case 7:
        mSCR = val;
        break;

    default:
        break;
    }
}

uint8_t UartPC16550D::readByte(const Address &addr)
{
    uint8_t val = 0;
    switch (addr.getOffset())
    {
    case 0:
        if (mLCR & DLAB)
        {
            // Divisor latch access bit set. Read DLL.
            val = mDLL;
        }
        else if (mFCR & FIFO_ENABLE)
        {
            // Receiver FIFO is enabled.
            if (!mXmitFifo.empty())
            {
                val = mRcvrFifo.front();
                mRcvrFifo.pop_front();
                if (mRcvrFifo.empty())
                {
                    mLSR &= ~DR; // clear data ready bit
                }
                return val;
            }
        }
        else
        {
            // Read buffer
            mLSR &= ~DR; // clear data ready bit
            val = mRBR;
        }
        break;

    case 1:
        if (mLCR & DLAB)
        {
            // Divisor latch access bit set. Read DLM.
            val = mDLM;
        }
        else
        {
            val = mIER;
        }
        break;

    case 2:
        val = mIIR;
        break;

    case 3:
        val = mLCR;
        break;

    case 4:
        val = mMCR;
        break;

    case 5:
        val = mLSR;
        // Clear bits that get cleared on read.
        mLSR &= ~(OE | PE | FE | BI | FIFO_ERR);
        break;

    case 6:
        val = mMSR;
        mMSR = 0xF0; // delta bits cleared on read
        break;

    case 7:
        val = mSCR;
        break;

    default:
        break;
    }

    return val;
}

bool UartPC16550D::decodeAddress(const Address &in, Address &out)
{
    uint32_t addr = in.getAbsolute() - mBase;
    out = Address((addr >> 16) & 0xFF, addr & 0xFFFF);
    return addr < 8; // 3 address lines
}

void UartPC16550D::addCycles(int cycles)
{
    mClocksUntilSend -= cycles;
    if (mClocksUntilSend > 0)
        return;

    // Reset for next character then send the current.
    mClocksUntilSend += mClocksPerByte;

    if (!(mLSR & THRE))
    {
        // There is something to transmit.
        uint8_t val;
        if (mFCR & FIFO_ENABLE)
        {
            val = mXmitFifo.front();
            mXmitFifo.pop_front();
            if (mXmitFifo.empty())
            {
                mLSR |= THRE | TEMT;
            }
        }
        else
        {
            val = mTHR;
            mLSR |= THRE | TEMT;
        }
        Log::trc(LOG_TAG).str("Transmitting ").hex(val, 2).show();

        if (mTerm)
        {
            // Write the character to the terminal.
            mTerm->write(val);
        }
    }

    // Check for something to read
    if (mTerm)
    {
        uint8_t val = mTerm->read();
        if (val)
        {
            receive(val);
        }
    }

    checkForInterrupts();
}

void UartPC16550D::checkForInterrupts()
{
    if (!mIER)
        return; // If no bits set, no interrupts allowed

    // TODO Implement
}

void UartPC16550D::setByteRate()
{
    // Baud rate = frequency / (divisor * 16)
    // Clocks per character = divisor * 2
    uint16_t divisor = ((uint16_t)mDLM << 8) | mDLL;
    mClocksPerByte = (uint32_t)divisor * 2;
}

void UartPC16550D::send()
{
    if (mMCR & LOOPBACK)
    {
        uint8_t val;
        if (mFCR & FIFO_ENABLE)
        {
            val = mXmitFifo.front();
            mXmitFifo.pop_front();
            if (mXmitFifo.empty())
            {
                mLSR |= THRE | TEMT;
            }
        }
        else
        {
            val = mTHR;
            mLSR |= THRE | TEMT;
        }
        receive(val);
        checkForInterrupts();
    }
    else
    {
        mClocksUntilSend = mClocksPerByte;
    }
}

void UartPC16550D::receive(uint8_t val)
{
    if (mFCR & FIFO_ENABLE)
    {
        mRcvrFifo.push_back(val);
        mLSR |= DR;
        if (mRcvrFifo.size() > FIFO_SIZE)
        {
            mLSR |= OE;
            mRcvrFifo.pop_back();
        }
    }
    else
    {
        if (rbrFull)
        {
            // Set RBR not read before filled error
            mLSR |= OE;
        }
        mRBR = val;
        mLSR |= DR;
        rbrFull = true;
    }
}

void UartPC16550D::setMSR(uint8_t mask, bool set)
{
    if (set)
    {
        mMSR |= mask;
    }
    else
    {
        mMSR &= ~mask;
    }
}
