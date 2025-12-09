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

#include "Slaves/Switches/DS2413/DS2413.h"

using OneWire::DS2413;
using OneWire::OneWireMaster;

enum DS2413_CMDS
{
    PIO_ACCESS_READ = 0xF5,
    PIO_ACCESS_WRITE = 0x5A
};

enum DS2413_PIO
{
    PIOA,
    PIOB,
    PIOAB
};

DS2413::DS2413(RandomAccessRomIterator &selector) : OneWireSlave(selector)
{
}

DS2413::CmdResult DS2413::pioAccessReadChA(uint8_t & val)
{
    DS2413::CmdResult result = OpFailure;

    result = pioAccessRead(val);

    val = (val & 0x01);

    return result;
}

DS2413::CmdResult DS2413::pioAccessReadChB(uint8_t & val)
{
    DS2413::CmdResult result = OpFailure;

    result = pioAccessRead(val);

    val = ((val >> 2) & 0x01);

    return result;
}

DS2413::CmdResult DS2413::pioAccessWriteChA(uint8_t val)
{
    DS2413::CmdResult result = OpFailure;

    uint8_t local_val = 0;

    //read current state of PIO
    result = pioAccessRead(local_val);

    if (result == DS2413::Success)
    {
        //modify
        //current state of pioB OR
        //desired state of pioA OR
        //bits[7:2] should all be 1, per datasheet
        val = (0xFC | (((local_val >> 1) & 0x02) | (0x01 & val)));

        //write, bit[1:0] new state of pio
        result = pioAccessWrite(val);
    }

    return result;
}

DS2413::CmdResult DS2413::pioAccessWriteChB(uint8_t val)
{
    DS2413::CmdResult result = OpFailure;

    uint8_t local_val = 0;

    //read current state of PIO
    result = pioAccessRead(local_val);

    if (result == DS2413::Success)
    {
        //modify
        //current state of pioA OR
        //desired state of pioB OR
        //bits[7:2] should all be 1, per datasheet
        val = (0xFC | ((local_val & 0x01) | (0x02 & (val << 1))));

        //write, bit[1:0] new state of pio
        result = pioAccessWrite(val);
    }

    return result;
}

DS2413::CmdResult DS2413::pioAccessWriteChAB(uint8_t val)
{
    return pioAccessWrite((0x03 & val) | 0xFC);
}

DS2413::CmdResult DS2413::pioAccessRead(uint8_t & val)
{
    DS2413::CmdResult result = DS2413::OpFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        ow_result = master().OWWriteByte(PIO_ACCESS_READ);

        if (ow_result == OneWireMaster::Success)
        {
            ow_result = master().OWReadByte(val);

            if (ow_result == OneWireMaster::Success)
            {
                result = DS2413::Success;
            }
            else
            {
                result = DS2413::CommsReadError;
            }
        }
        else
        {
            result = DS2413::CommsWriteError;
        }
    }

    return result;
}

DS2413::CmdResult DS2413::pioAccessWrite(uint8_t val)
{
    DS2413::CmdResult result = DS2413::OpFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        uint8_t send_block[] = { PIO_ACCESS_WRITE, val, static_cast<uint8_t>(~val) };

        ow_result = master().OWWriteBlock(send_block, 3);
        if (ow_result == OneWireMaster::Success)
        {
            uint8_t expected_status = ((0x01 & val) | ((0x01 & val) << 1) |
                ((0x02 & val) << 1) | ((0x02 & val) << 2));

            uint8_t rcv_block[2];
            ow_result = master().OWReadBlock(rcv_block, 2);

            if (ow_result == OneWireMaster::Success)
            {
                if ((rcv_block[0] == 0xAA) && ((rcv_block[1] & 0x0F) == expected_status))
                {
                    result = DS2413::Success;
                }
            }
            else
            {
                result = DS2413::CommsReadError;
            }
        }
        else
        {
            result = DS2413::CommsWriteError;
        }
    }

    return result;
}
