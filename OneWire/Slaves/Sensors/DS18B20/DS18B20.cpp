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


#include "Slaves/Sensors/DS18B20/DS18B20.h"
#include "mbed_wait_api.h"


using namespace OneWire;
using namespace OneWire::crc;


enum DS18B20_CMDS
{
    WRITE_SCRATCHPAD = 0x4E,
    READ_SCRATCHPAD = 0xBE,
    COPY_SCRATCHPAD = 0x48,
    CONV_TEMPERATURE = 0x44,
    READ_POWER_SUPPY = 0xB4,
    RECALL = 0xB8  
};


/**********************************************************************/
DS18B20::DS18B20(RandomAccessRomIterator &selector):OneWireSlave(selector)
{
}


/**********************************************************************/
OneWireSlave::CmdResult DS18B20::writeScratchPad(uint8_t th, uint8_t tl, Resolution res)
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        uint8_t sendBlock[] = {WRITE_SCRATCHPAD, th, tl, res};
        
        owmResult = master().OWWriteBlock(sendBlock, 4);
        if (owmResult == OneWireMaster::Success)
        {
            deviceResult = OneWireSlave::Success;
        }
        else
        {
            deviceResult = OneWireSlave::CommunicationError;
        }
    }
    
    return deviceResult;
}


/**********************************************************************/
OneWireSlave::CmdResult DS18B20::readScratchPad(uint8_t * scratchPadBuff)
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        owmResult = master().OWWriteByteSetLevel(READ_SCRATCHPAD, OneWireMaster::NormalLevel);
        if (owmResult == OneWireMaster::Success)
        {
            uint8_t rxBlock[9];
            owmResult = master().OWReadBlock(rxBlock, 9);
            
            uint8_t crcCheck = calculateCrc8(rxBlock, 8);
            if ((owmResult == OneWireMaster::Success) && (crcCheck == rxBlock[8]))
            {
                std::memcpy(scratchPadBuff, rxBlock, 8);
                deviceResult = OneWireSlave::Success;
            }
            else
            {
                deviceResult = OneWireSlave::CommunicationError;
            }
        }
        else
        {
            deviceResult = OneWireSlave::CommunicationError;
        }
    }
    
    return deviceResult;
}


/**********************************************************************/
OneWireSlave::CmdResult DS18B20::readPowerSupply(bool & localPower)
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    if (owmResult == OneWireMaster::Success)
    {
        owmResult = master().OWWriteByteSetLevel(READ_POWER_SUPPY, OneWireMaster::NormalLevel);
        if(owmResult == OneWireMaster::Success)
        {
            uint8_t rtnBit = 0;
            
            owmResult = master().OWTouchBitSetLevel(rtnBit, OneWireMaster::NormalLevel);
            if(owmResult == OneWireMaster::Success)
            {
                localPower = (rtnBit & 0x01);
                deviceResult = OneWireSlave::Success;
            }
            else
            {
                deviceResult = OneWireSlave::CommunicationError;
            }
        }
        else
        {
            deviceResult = OneWireSlave::CommunicationError;
        }
    }
    
    return deviceResult;
}

/**********************************************************************/
OneWireSlave::CmdResult DS18B20::copyScratchPad( void )
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    bool hasLocalPower = false;
    deviceResult = this->readPowerSupply(hasLocalPower);
    
    if(deviceResult == OneWireSlave::Success)
    {
        OneWireMaster::CmdResult owmResult = selectDevice();
        
        if (owmResult == OneWireMaster::Success)
        {
            if(hasLocalPower)
            {
                owmResult = master().OWWriteByteSetLevel(COPY_SCRATCHPAD, OneWireMaster::NormalLevel); 
                if (owmResult == OneWireMaster::Success)
                {
                    uint8_t recvbit = 0;
                    do
                    {
                        owmResult = master().OWTouchBitSetLevel(recvbit, OneWireMaster::NormalLevel);
                    }
                    while((!recvbit) && (owmResult == OneWireMaster::Success));
                    
                    if ((owmResult == OneWireMaster::Success) && (recvbit & 1))
                    {
                        deviceResult = OneWireSlave::Success;
                    }
                    else
                    {
                        deviceResult = OneWireSlave::TimeoutError;
                    }
                }
                else
                {
                    deviceResult = OneWireSlave::CommunicationError;
                }
            }
            else
            {
                owmResult = master().OWWriteByteSetLevel(COPY_SCRATCHPAD, OneWireMaster::StrongLevel); 
                if (owmResult == OneWireMaster::Success)
                {
                    wait_us(1000*10);
                    
                    owmResult = master().OWSetLevel(OneWireMaster::NormalLevel);  
                    if (owmResult == OneWireMaster::Success)
                    {
                        deviceResult = OneWireSlave::Success;
                    }
                    else
                    {
                        deviceResult = OneWireSlave::CommunicationError;
                    }
                }
                else
                {
                    deviceResult = OneWireSlave::CommunicationError;
                }
            }
        }
        else
        {
            deviceResult = OneWireSlave::OperationFailure;
        }
    }
    
    return deviceResult;
}

/**********************************************************************/
OneWireSlave::CmdResult DS18B20::convertTemperature(float & temp)
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    bool hasLocalPower = false;
    deviceResult = this->readPowerSupply(hasLocalPower);
    
    uint8_t scratchPadBuff[8];
    
    if (deviceResult == OneWireSlave::Success)
    {
        OneWireMaster::CmdResult owmResult = selectDevice();
        if(owmResult == OneWireMaster::Success)
        {
            if(hasLocalPower)
            {
                owmResult = master().OWWriteByteSetLevel(CONV_TEMPERATURE, OneWireMaster::NormalLevel); 
                if (owmResult == OneWireMaster::Success)
                {
                    uint8_t recvbit = 0;
                    do
                    {
                        owmResult = master().OWTouchBitSetLevel(recvbit, OneWireMaster::NormalLevel);
                    }
                    while((owmResult == OneWireMaster::Success) && (!recvbit));
                    
                    if((owmResult == OneWireMaster::Success) && (recvbit & 1))
                    {
                        deviceResult = this->readScratchPad(scratchPadBuff);
                    }
                    else
                    {
                        deviceResult = OneWireSlave::TimeoutError;
                    }
                }
                else
                {
                    deviceResult = OneWireSlave::CommunicationError;
                }
            }
            else
            {
                owmResult = master().OWWriteByteSetLevel(CONV_TEMPERATURE, OneWireMaster::StrongLevel); 
                if (owmResult == OneWireMaster::Success)
                {
                    wait_us(1000*750);
                    
                    owmResult = master().OWSetLevel(OneWireMaster::NormalLevel);  
                    if (owmResult == OneWireMaster::Success)
                    {
                        deviceResult = this->readScratchPad(scratchPadBuff);
                    }
                    else
                    {
                        deviceResult = OneWireSlave::CommunicationError;
                    }
                }
                else
                {
                    deviceResult = OneWireSlave::CommunicationError;
                }
            }
        }
        else
        {
            deviceResult = OneWireSlave::OperationFailure;
        }
    }
    
    if(deviceResult == OneWireSlave::Success)
    {
        int16_t intTemp = ((scratchPadBuff[1] << 8) | scratchPadBuff[0]);
        
        switch(scratchPadBuff[4])
        {
            case DS18B20::NineBit:
                temp = (intTemp * 0.5F);
            break;
            
            case DS18B20::TenBit:
                temp = (intTemp * 0.25F);
            break;
            
            case DS18B20::ElevenBit:
                temp = (intTemp * 0.125F);
            break;
            
            case DS18B20::TwelveBit:
                temp = (intTemp * 0.0625F);
            break;
            
            default:
                deviceResult = OneWireSlave::OperationFailure;
            break;
        }
    }
    
    return deviceResult;
}


/**********************************************************************/
OneWireSlave::CmdResult DS18B20::recallEEPROM( void )
{
    OneWireSlave::CmdResult deviceResult = OneWireSlave::OperationFailure;
    
    OneWireMaster::CmdResult owmResult = selectDevice();
    
    if (owmResult == OneWireMaster::Success)
    {
        uint8_t cmd = RECALL;
        
        owmResult = master().OWWriteBlock(&cmd, 1);
        if (owmResult == OneWireMaster::Success)
        {
            deviceResult = OneWireSlave::Success;
        }
        else
        {
            deviceResult = OneWireSlave::CommunicationError;
        }
    }
    
    return deviceResult;
}
