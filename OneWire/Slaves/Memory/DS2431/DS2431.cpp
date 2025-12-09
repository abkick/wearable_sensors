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

#include "mbed_wait_api.h"
#include "Slaves/Memory/DS2431/DS2431.h"

using namespace OneWire;
using namespace OneWire::crc;

enum Command
{
    WriteScratchpad = 0x0F,
    ReadScratchpad = 0xAA,
    CopyScratchpad = 0x55,
    ReadMemory = 0xF0
};

static const DS2431::Address beginReservedAddress = 0x0088;

//*********************************************************************
DS2431::DS2431(RandomAccessRomIterator & selector) : OneWireSlave(selector)
{
}

//*********************************************************************
OneWireSlave::CmdResult DS2431::writeMemory(Address targetAddress, const Scratchpad & data)
{    
    if (((targetAddress & 0x7) != 0x0) || ((targetAddress + data.size()) > beginReservedAddress))
    {
        return OneWireSlave::OperationFailure;
    }
    OneWireSlave::CmdResult result = writeScratchpad(targetAddress, data);
    if (result != OneWireSlave::Success)
    {
        return result;
    }
    Scratchpad readData;
    uint8_t esByte;
    result = readScratchpad(readData, esByte);
    if (result != OneWireSlave::Success)
    {
        return result;
    }
    result = copyScratchpad(targetAddress, esByte);    
    return result;
}

//*********************************************************************
OneWireSlave::CmdResult DS2431::readMemory(Address targetAddress, uint8_t numBytes, uint8_t * data)
{    
    if ((targetAddress + numBytes) > beginReservedAddress)
    {
        return OneWireSlave::OperationFailure;
    }
    OneWireMaster::CmdResult owmResult = selectDevice();
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t sendBlock[] = { ReadMemory, static_cast<uint8_t>(targetAddress), static_cast<uint8_t>(targetAddress >> 8) };
    owmResult = master().OWWriteBlock(sendBlock, sizeof(sendBlock) / sizeof(sendBlock[0]));
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    owmResult = master().OWReadBlock(data, numBytes);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    // else
    return OneWireSlave::Success;
}

//*********************************************************************
OneWireSlave::CmdResult DS2431::writeScratchpad(Address targetAddress, const Scratchpad & data)
{    
    OneWireMaster::CmdResult owmResult = selectDevice();
    if(owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t sendBlock[3 + Scratchpad::csize] = { WriteScratchpad, static_cast<uint8_t>(targetAddress), static_cast<uint8_t>(targetAddress >> 8) };
    std::memcpy((sendBlock + 3), data.data(), data.size());
    owmResult = master().OWWriteBlock(sendBlock, sizeof(sendBlock) / sizeof(sendBlock[0]));
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t recvbyte;
    owmResult = master().OWReadByte(recvbyte);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint16_t invCRC16 = recvbyte;
    owmResult = master().OWReadByte(recvbyte);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    invCRC16 |= (recvbyte << 8); 
    //calc our own inverted CRC16 to compare with one returned
    uint16_t calculatedInvCRC16 = ~calculateCrc16(sendBlock, sizeof(sendBlock) / sizeof(sendBlock[0]));
    if (invCRC16 != calculatedInvCRC16)
    {
        return OneWireSlave::CrcError;
    }
    // else
    return OneWireSlave::Success;
}

//*********************************************************************
OneWireSlave::CmdResult DS2431::readScratchpad(Scratchpad & data, uint8_t & esByte)
{    
    OneWireMaster::CmdResult owmResult = selectDevice();
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    owmResult = master().OWWriteByte(ReadScratchpad);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t recvBlock[13];
    const size_t recvBlockSize = sizeof(recvBlock) / sizeof(recvBlock[0]);
    owmResult = master().OWReadBlock(recvBlock, recvBlockSize);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint16_t invCRC16 = ((recvBlock[recvBlockSize - 1] << 8) | recvBlock[recvBlockSize - 2]);
    // Shift contents down
    std::memmove(recvBlock + 1, recvBlock, recvBlockSize - 1);
    recvBlock[0] = ReadScratchpad;
    //calc our own inverted CRC16 to compare with one returned
    uint16_t calculatedInvCRC16 = ~calculateCrc16(recvBlock, recvBlockSize - 1);
    if (invCRC16 != calculatedInvCRC16)
    {
        return OneWireSlave::CrcError;
    }
    esByte = recvBlock[3];
    std::memcpy(data.data(), (recvBlock + 4), data.size());
    return OneWireSlave::Success;
}

//*********************************************************************
OneWireSlave::CmdResult DS2431::copyScratchpad(Address targetAddress, uint8_t esByte)
{    
    OneWireMaster::CmdResult owmResult = selectDevice();
    if(owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t sendBlock[] = { CopyScratchpad, static_cast<uint8_t>(targetAddress), static_cast<uint8_t>(targetAddress >> 8) };
    owmResult = master().OWWriteBlock(sendBlock, sizeof(sendBlock) / sizeof(sendBlock[0]));
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    owmResult = master().OWWriteByteSetLevel(esByte, OneWireMaster::StrongLevel);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    wait_us(1000*10);
    owmResult = master().OWSetLevel(OneWireMaster::NormalLevel);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    uint8_t check;
    owmResult = master().OWReadByte(check);
    if (owmResult != OneWireMaster::Success)
    {
        return OneWireSlave::CommunicationError;
    }
    if (check != 0xAA)
    {
        return OneWireSlave::OperationFailure;
    }
    // else
    return OneWireSlave::Success;
}
