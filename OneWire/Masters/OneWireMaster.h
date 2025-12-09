/******************************************************************//**
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
**********************************************************************/

#ifndef OneWire_Masters_OneWireMaster
#define OneWire_Masters_OneWireMaster

#include <stdint.h>
#include <stddef.h>

namespace OneWire
{
    /// Base class for all 1-Wire Masters.
    class OneWireMaster
    {
    public:
        /// Speed of the 1-Wire bus.
        enum OWSpeed
        {
            StandardSpeed = 0x00,
            OverdriveSpeed = 0x01
        };

        /// Level of the 1-Wire bus.
        enum OWLevel
        {
            NormalLevel = 0x00,
            StrongLevel = 0x02
        };

        /// Search direction for the triplet operation.
        enum SearchDirection
        {
            WriteZero = 0,
            WriteOne = 1
        };

        /// Result of all 1-Wire commands.
        enum CmdResult
        {
            Success,
            CommunicationWriteError,
            CommunicationReadError,
            TimeoutError,
            OperationFailure
        };

        /// Allow freeing through a base class pointer.
        virtual ~OneWireMaster() { }

        /// Initialize a master for use.
        virtual CmdResult OWInitMaster() = 0;

        /// Reset all of the devices on the 1-Wire bus and check for a presence pulse.
        /// @returns OperationFailure if reset was performed but no presence pulse was detected.
        virtual CmdResult OWReset() = 0;

        /// Send and receive one bit of communication and set a new level on the 1-Wire bus.
        /// @param[in,out] sendRecvBit Buffer containing the bit to send on 1-Wire bus in lsb.
        ///                            Read data from 1-Wire bus will be returned in lsb.
        /// @param afterLevel Level to set the 1-Wire bus to after communication.
        virtual CmdResult OWTouchBitSetLevel(uint8_t & sendRecvBit, OWLevel afterLevel) = 0;

        /// Send one byte of communication and set a new level on the 1-Wire bus.
        /// @param sendByte Byte to send on the 1-Wire bus.
        /// @param afterLevel Level to set the 1-Wire bus to after communication.
        virtual CmdResult OWWriteByteSetLevel(uint8_t sendByte, OWLevel afterLevel) = 0;

        /// Receive one byte of communication and set a new level on the 1-Wire bus.
        /// @param recvByte Buffer to receive the data from the 1-Wire bus.
        /// @param afterLevel Level to set the 1-Wire bus to after communication.
        virtual CmdResult OWReadByteSetLevel(uint8_t & recvByte, OWLevel afterLevel) = 0;

        /// Send a block of communication on the 1-Wire bus.
        /// @param[in] sendBuf Buffer to send on the 1-Wire bus.
        /// @param sendLen Length of the buffer to send.
        virtual CmdResult OWWriteBlock(const uint8_t *sendBuf, size_t sendLen);

        /// Receive a block of communication on the 1-Wire bus.
        /// @param[out] recvBuf Buffer to receive the data from the 1-Wire bus.
        /// @param recvLen Length of the buffer to receive.
        virtual CmdResult OWReadBlock(uint8_t *recvBuf, size_t recvLen);

        /// Set the 1-Wire bus communication speed.
        virtual CmdResult OWSetSpeed(OWSpeed newSpeed) = 0;

        /// Set the 1-Wire bus level.
        virtual CmdResult OWSetLevel(OWLevel newLevel) = 0;

        /**********************************************************//**
        * @brief 1-Wire Triplet operation.
        *
        * @details Perform one bit of a 1-Wire search. This command
        * does two read bits and one write bit. The write bit is either
        * the default direction (all device have same bit) or in case
        * of a discrepancy, the 'searchDirection' parameter is used.
        *
        * @param[in,out] searchDirection
        * Input with desired direction in case both read bits are zero.
        * Output with direction taken based on read bits.
        *
        * @param[out] sbr Bit result of first read operation.
        * @param[out] tsb Bit result of second read operation.
        **************************************************************/
        virtual CmdResult OWTriplet(SearchDirection & searchDirection, uint8_t & sbr, uint8_t & tsb);

        /// Send one bit of communication and set a new level on the 1-Wire bus.
        /// @param sendBit Buffer containing the bit to send on 1-Wire bus in lsb.
        /// @param afterLevel Level to set the 1-Wire bus to after communication.
        CmdResult OWWriteBitSetLevel(uint8_t sendBit, OWLevel afterLevel) { return OWTouchBitSetLevel(sendBit, afterLevel); }

        /// Receive one bit of communication and set a new level on the 1-Wire bus.
        /// @param[out] sendRecvBit Read data from 1-Wire bus will be returned in lsb.
        /// @param afterLevel Level to set the 1-Wire bus to after communication.
        CmdResult OWReadBitSetLevel(uint8_t & recvBit, OWLevel afterLevel) { recvBit = 0x01; return OWTouchBitSetLevel(recvBit, afterLevel); }

        // Alternate forms of read and write functions
        CmdResult OWWriteBit(uint8_t sendBit) { return OWWriteBitSetLevel(sendBit, NormalLevel); }
        CmdResult OWReadBit(uint8_t & recvBit) { return OWReadBitSetLevel(recvBit, NormalLevel); }
        CmdResult OWWriteBitPower(uint8_t sendBit) { return OWWriteBitSetLevel(sendBit, StrongLevel); }
        CmdResult OWReadBitPower(uint8_t & recvBit) { return OWReadBitSetLevel(recvBit, StrongLevel); }
        CmdResult OWWriteByte(uint8_t sendByte) { return OWWriteByteSetLevel(sendByte, NormalLevel); }
        CmdResult OWReadByte(uint8_t & recvByte) { return OWReadByteSetLevel(recvByte, NormalLevel); }
        CmdResult OWWriteBytePower(uint8_t sendByte) { return OWWriteByteSetLevel(sendByte, StrongLevel); }
        CmdResult OWReadBytePower(uint8_t & recvByte) { return OWReadByteSetLevel(recvByte, StrongLevel); }
    };
}

#endif
