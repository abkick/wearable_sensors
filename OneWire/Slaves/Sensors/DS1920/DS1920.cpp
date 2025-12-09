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


#include "Slaves/Sensors/DS1920/DS1920.h"
#include "mbed_wait_api.h"


using namespace OneWire;
using namespace OneWire::crc;


enum DS1920_CMDS
{
    WRITE_SCRATCHPAD = 0x4E,
    READ_SCRATCHPAD = 0xBE,
    COPY_SCRATCHPAD = 0x48,
    CONV_TEMPERATURE = 0x44,
    RECALL = 0xB8  
};


/**********************************************************************/
DS1920::DS1920(RandomAccessRomIterator &selector):OneWireSlave(selector)
{
}


/**********************************************************************/
DS1920::CmdResult DS1920::writeScratchPad(uint8_t th, uint8_t tl)
{
    DS1920::CmdResult deviceResult = DS1920::OpFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        uint8_t sendBlock[] = {WRITE_SCRATCHPAD, th, tl};
        
        owmResult = master().OWWriteBlock(sendBlock, 3);
        if (owmResult == OneWireMaster::Success)
        {
            deviceResult = DS1920::Success;
        }
        else
        {
            deviceResult = DS1920::CommsWriteError;
        }
    }
    
    return deviceResult;
}


/**********************************************************************/
DS1920::CmdResult DS1920::readScratchPad(uint8_t * scratchPadBuff)
{
    DS1920::CmdResult deviceResult = DS1920::OpFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        uint8_t cmd = READ_SCRATCHPAD;
        
        owmResult = master().OWWriteBlock(&cmd, 1);
        if (owmResult == OneWireMaster::Success)
        {
            uint8_t rxBlock[9];
            owmResult = master().OWReadBlock(rxBlock, 9);
            
            uint8_t crcCheck = calculateCrc8(rxBlock, 8);
            if ((owmResult == OneWireMaster::Success) && (crcCheck == rxBlock[8]))
            {
                std::memcpy(scratchPadBuff, rxBlock, 8);
                deviceResult = DS1920::Success;
            }
            else
            {
                deviceResult = DS1920::CommsReadError;
            }
        }
        else
        {
            deviceResult = DS1920::CommsWriteError;
        }
    }
    
    return deviceResult;
}

/**********************************************************************/
DS1920::CmdResult DS1920::copyScratchPad( void )
{
    DS1920::CmdResult deviceResult = DS1920::OpFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        owmResult = master().OWWriteByteSetLevel(COPY_SCRATCHPAD, OneWireMaster::StrongLevel); 
        if (owmResult == OneWireMaster::Success)
        {
            wait_us(1000*10);
            
            owmResult = master().OWSetLevel(OneWireMaster::NormalLevel);  
            if (owmResult == OneWireMaster::Success)
            {
                deviceResult = DS1920::Success;
            }
        }
        else
        {
            deviceResult = DS1920::CommsWriteError;
        }
    }
    
    return deviceResult;
}

/**********************************************************************/
DS1920::CmdResult DS1920::convertTemperature(float & temp)
{
    DS1920::CmdResult deviceResult = DS1920::OpFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        owmResult = master().OWWriteByteSetLevel(CONV_TEMPERATURE, OneWireMaster::StrongLevel); 
        if (owmResult == OneWireMaster::Success)
        {
            wait_us(1000*750);
            
            owmResult = master().OWSetLevel(OneWireMaster::NormalLevel);  
            if (owmResult == OneWireMaster::Success)
            {
                uint8_t scratchPadBuff[8];
                deviceResult = this->readScratchPad(scratchPadBuff);
                if(deviceResult == DS1920::Success)
                {
                    if(scratchPadBuff[0] & 1)
                    {
                        temp = (((float) (scratchPadBuff[0] >> 1)) + 0.5F);
                    }
                    else
                    {
                        temp = ((float) (scratchPadBuff[0] >> 1));
                    }
                    
                    if(scratchPadBuff[1])
                    {
                        temp = (temp * -1.0F);
                    }
                }
            }
        }
        else
        {
            deviceResult = DS1920::CommsWriteError;
        }
    }
    
    return deviceResult;
}


/**********************************************************************/
DS1920::CmdResult DS1920::recallEEPROM( void )
{
    DS1920::CmdResult deviceResult = DS1920::OpFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        uint8_t cmd = RECALL;
        
        owmResult = master().OWWriteBlock(&cmd, 1);
        if (owmResult == OneWireMaster::Success)
        {
            deviceResult = DS1920::Success;
        }
        else
        {
            deviceResult = DS1920::CommsWriteError;
        }
    }
    
    return deviceResult;
}
