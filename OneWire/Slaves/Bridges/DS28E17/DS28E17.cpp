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

#include "Masters/OneWireMaster.h"
#include "Slaves/Bridges/DS28E17/DS28E17.h"
#include "Utilities/crc.h"

using OneWire::DS28E17;
using OneWire::OneWireMaster;
using namespace OneWire::crc;

enum Command
{
    WriteDataWithStopCmd = 0x4B,
    WriteDataNoStopCmd = 0x5A,
    WriteDataOnlyCmd = 0x69,
    WriteDataOnlyWithStopCmd = 0x78,
    ReadDataWithStopCmd = 0x87,
    WriteReadDataWithStopCmd = 0x2D,
    WriteConfigurationCmd = 0xD2,
    ReadConfigurationCmd = 0xE1,
    EnableSleepModeCmd = 0x1E,
    ReadDeviceRevisionCmd = 0xC3
};


//*********************************************************************
DS28E17::DS28E17(RandomAccessRomIterator &selector)
    : OneWireSlave(selector)
{

}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeDataWithStop(uint8_t I2C_addr, uint8_t length,
                                                  uint8_t *data, uint8_t &status,
                                                  uint8_t &wr_status)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        //seed the crc
        uint16_t crc16 = 0;

        // Form the 1-Wire Packet 

        // I2C Write Data with Stop command
        send_block[send_cnt] = WriteDataWithStopCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // I2C Address
        send_block[send_cnt] = I2C_addr;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Length field
        send_block[send_cnt] = length;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the write data  
        for (size_t idx = 0; idx < length; idx++)
        {
            send_block[send_cnt] = data[idx];
            crc16 = calculateCrc16(crc16, send_block[send_cnt++]);
        }

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = ((uint8_t)(crc16 & 0xFF));
        send_block[send_cnt++] = ((uint8_t)((crc16 >> 8) & 0xFF));

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status, wr_status);
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeDataNoStop(uint8_t I2C_addr, uint8_t length,
                                                uint8_t *data, uint8_t &status,
                                                uint8_t &wr_status)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // seed the crc
        uint16_t crc16 = 0;

        // I2C Write Data with Stop command
        send_block[send_cnt] = WriteDataNoStopCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // I2C Address
        send_block[send_cnt] = I2C_addr;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Length field
        send_block[send_cnt] = length;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the write data
        for (size_t idx = 0; idx < length; idx++)
        {
            send_block[send_cnt] = data[idx];
            crc16 = calculateCrc16(crc16, send_block[send_cnt++]);
        }

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = ((uint8_t)(crc16 & 0xFF));
        send_block[send_cnt++] = ((uint8_t)((crc16 >> 8) & 0xFF));

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status, wr_status);
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeDataOnly(uint8_t length, uint8_t *data,
                                              uint8_t &status, uint8_t &wr_status)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // seed the crc
        uint16_t crc16 = 0;

        // Form the 1-Wire Packet

        // I2C Write Data with Stop command
        send_block[send_cnt] = WriteDataOnlyCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Length field
        send_block[send_cnt] = length;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the write data
        for (size_t idx = 0; idx < length; idx++)
        {
            send_block[send_cnt] = data[idx];
            crc16 = calculateCrc16(crc16, send_block[send_cnt++]);
        }

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = (crc16 & 0xFF);
        send_block[send_cnt++] = ((crc16 >> 8) & 0xFF);

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status, wr_status);
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeDataOnlyWithStop(uint8_t length, uint8_t *data,
                                                      uint8_t &status, uint8_t &wr_status)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        //seed the crc
        uint16_t crc16 = 0;

        // Form the 1-Wire Packet

        // I2C Write Data with Stop command
        send_block[send_cnt] = WriteDataOnlyWithStopCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Length field
        send_block[send_cnt] = length;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the write data
        for (size_t idx = 0; idx < length; idx++)
        {
            send_block[send_cnt] = data[idx];
            crc16 = calculateCrc16(crc16, send_block[send_cnt++]);
        }

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = (crc16 & 0xFF);
        send_block[send_cnt++] = ((crc16 >> 8) & 0xFF);

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status, wr_status);
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeReadDataWithStop(uint8_t I2C_addr, uint8_t length,
                                                      uint8_t *data, uint8_t nu_bytes_read,
                                                      uint8_t &status, uint8_t &wr_status,
                                                      uint8_t *read_data)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    size_t idx = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        //seed the crc
        uint16_t crc16 = 0;

        // Form the 1-Wire Packet

        // I2C Write Data with Stop command
        send_block[send_cnt] = WriteReadDataWithStopCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // I2C Address
        send_block[send_cnt] = I2C_addr;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Length field
        send_block[send_cnt] = length;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the write data
        for (idx = 0; idx < length; idx++)
        {
            send_block[send_cnt] = data[idx];
            crc16 = calculateCrc16(crc16, send_block[send_cnt++]);
        }

        // # of bytes to Read field
        send_block[send_cnt] = nu_bytes_read;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = (crc16 & 0xFF);
        send_block[send_cnt++] = ((crc16 >> 8) & 0xFF);

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status, wr_status);
        if (bridge_result == DS28E17::Success)
        {
            ow_result = master().OWReadBlock(read_data, nu_bytes_read);
            if (ow_result == OneWireMaster::Success)
            {
                bridge_result = DS28E17::Success;
            }
            else
            {
                bridge_result = DS28E17::CommsReadBlockError;
            }
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::readDataWithStop(uint8_t I2C_addr, uint8_t nu_bytes_read,
                                                 uint8_t &status, uint8_t *read_data)
{
    DS28E17::CmdResult  bridge_result = DS28E17::OperationFailure;

    size_t send_cnt = 0;
    uint8_t send_block[0xff];

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        //seed the crc for transmit bytes
        uint16_t crc16 = 0;

        // Form the 1-Wire Packet to send

        // I2C Write Data with Stop command
        send_block[send_cnt] = ReadDataWithStopCmd;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // I2C Address
        send_block[send_cnt] = I2C_addr;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // # of bytes to Read field
        send_block[send_cnt] = nu_bytes_read;
        crc16 = calculateCrc16(crc16, send_block[send_cnt++]);

        // Form the CRC16
        crc16 ^= 0xFFFF;
        send_block[send_cnt++] = (crc16 & 0xFF);
        send_block[send_cnt++] = ((crc16 >> 8) & 0xFF);

        // Send Packet
        bridge_result = sendPacket(send_block, send_cnt, status);
        if (bridge_result == DS28E17::Success)
        {
            ow_result = master().OWReadBlock(read_data, nu_bytes_read);
            if (ow_result == OneWireMaster::Success)
            {
                bridge_result = DS28E17::Success;
            }
            else
            {
                bridge_result = DS28E17::CommsReadBlockError;
            }
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::writeConfigReg(uint8_t data)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // Send CMD and Data
        uint8_t send_block[] = { WriteConfigurationCmd, data };

        ow_result = master().OWWriteBlock(send_block, 2);
        if (ow_result == OneWireMaster::Success)
        {
            bridge_result = DS28E17::Success;
        }
        else
        {
            bridge_result = DS28E17::CommsWriteBlockError;
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::readConfigReg(uint8_t & config)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // Send CMD and receive Data
        ow_result = master().OWWriteByte(ReadConfigurationCmd);
        if (ow_result == OneWireMaster::Success)
        {
            ow_result = master().OWReadByte(config);
            if (ow_result == OneWireMaster::Success)
            {
                bridge_result = DS28E17::Success;
            }
            else
            {
                bridge_result = DS28E17::CommsReadByteError;
            }
        }
        else
        {
            bridge_result = DS28E17::CommsWriteByteError;
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::enableSleepMode()
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // Send CMD
        ow_result = master().OWWriteByte(EnableSleepModeCmd);
        if (ow_result == OneWireMaster::Success)
        {
            bridge_result = DS28E17::Success;
        }
        else
        {
            bridge_result = DS28E17::CommsWriteByteError;
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::readDeviceRevision(uint8_t & rev)
{
    DS28E17::CmdResult bridge_result = DS28E17::OperationFailure;

    OneWireMaster::CmdResult ow_result = selectDevice();

    if (ow_result == OneWireMaster::Success)
    {
        // Send CMD and receive Data
        ow_result = master().OWWriteByte(ReadDeviceRevisionCmd);
        if (ow_result == OneWireMaster::Success)
        {
            ow_result = master().OWReadByte(rev);
            if (ow_result == OneWireMaster::Success)
            {
                bridge_result = DS28E17::Success;
            }
            else
            {
                bridge_result = DS28E17::CommsReadByteError;
            }
        }
        else
        {
            bridge_result = DS28E17::CommsWriteByteError;
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::sendPacket(const uint8_t * data, uint8_t data_length,
                                        uint8_t & status, uint8_t & wr_status)
{
    DS28E17::CmdResult bridge_result = DS28E17::CommsWriteBlockError;
    uint32_t poll_count = 0;

    OneWireMaster::CmdResult ow_result = master().OWWriteBlock(data, data_length);

    if (ow_result == OneWireMaster::Success)
    {
        // Poll for Zero 1-Wire bit and return if an error occurs
        uint8_t recvbit = 0x01;
        do
        {
            ow_result = master().OWReadBit(recvbit);
        } while (recvbit && (poll_count++ < pollLimit) && (ow_result == OneWireMaster::Success));

        if (ow_result == OneWireMaster::Success)
        {
            if (poll_count < pollLimit)
            {
                //Read Status and write status
                uint8_t read_block[2];

                ow_result = master().OWReadBlock(read_block, 2);

                if (ow_result == OneWireMaster::Success)
                {
                    status = read_block[0];
                    wr_status = read_block[1];
                    bridge_result = DS28E17::Success;
                }
                else
                {
                    bridge_result = DS28E17::CommsReadBlockError;
                }
            }
            else
            {
                bridge_result = DS28E17::TimeoutError;
            }
        }
        else
        {
            bridge_result = DS28E17::CommsReadBitError;
        }
    }

    return bridge_result;
}


//*********************************************************************
DS28E17::CmdResult DS28E17::sendPacket(const uint8_t * data, uint8_t data_length,
                                        uint8_t & status)
{
    DS28E17::CmdResult bridge_result = DS28E17::CommsWriteBlockError;
    uint32_t poll_count = 0;

    OneWireMaster::CmdResult ow_result = master().OWWriteBlock(data, data_length);

    if (ow_result == OneWireMaster::Success)
    {
        // Poll for Zero 1-Wire bit and return if an error occurs
        uint8_t recvbit = 0x01;
        do
        {
            ow_result = master().OWReadBit(recvbit);
        } while (recvbit && (poll_count++ < pollLimit) && (ow_result == OneWireMaster::Success));

        if (ow_result == OneWireMaster::Success)
        {
            if (poll_count < pollLimit)
            {
                //Read Status 
                ow_result = master().OWReadByte(status);
                if (ow_result == OneWireMaster::Success)
                {
                    bridge_result = DS28E17::Success;
                }
                else
                {
                    bridge_result = DS28E17::CommsReadByteError;
                }
            }
            else
            {
                bridge_result = DS28E17::TimeoutError;
            }
        }
        else
        {
            bridge_result = DS28E17::CommsReadBitError;
        }
    }

    return bridge_result;
}

