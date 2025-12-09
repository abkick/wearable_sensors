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

#ifndef OneWire_Bridge_DS28E17
#define OneWire_Bridge_DS28E17

#include <stdint.h>
#include <stddef.h>
#include "Slaves/OneWireSlave.h"

namespace OneWire
{
    class OneWireMaster;
    
    /**
    * @brief DS28E17 1-Wire®-to-I2C Master Bridge
    *
    * @details The DS28E17 is a 1-Wire slave to I2C master bridge 
    * device that interfaces directly to I2C slaves at standard 
    * (100kHz max) or fast (400kHz max). Data transfers serially by 
    * means of the 1-Wire® protocol, which requires only a single data 
    * lead and a ground return. Every DS28E17 is guaranteed to have a 
    * unique 64-bit ROM registration number that serves as a node 
    * address in the 1-Wire network. Multiple DS28E17 devices can 
    * coexist with other devices in the 1-Wire network and be accessed 
    * individually without affecting other devices. The DS28E17 allows 
    * using complex I2C devices such as display controllers, ADCs, DACs,
    * I2C sensors, etc. in a 1-Wire environment. Each self-timed DS28E17 
    * provides 1-Wire access for a single I2C interface.
    *
    */
    class DS28E17 : public OneWireSlave
    {
    public:
        
        ///Result of all operations
        enum CmdResult
        {
            Success,
            CommsReadBitError,
            CommsWriteBitError,
            CommsReadByteError,
            CommsWriteByteError,
            CommsReadBlockError,
            CommsWriteBlockError,
            TimeoutError,
            OperationFailure
        };

        /**********************************************************//**
        * @brief DS28E17 constructor
        *
        *
        * On Entry:
        * @param[in] selector - RandomAccessRomIterator object that 
        * encapsulates master associated with this device and rom control 
        * commands
        **************************************************************/
        DS28E17(RandomAccessRomIterator &selector);

        /**********************************************************//**
        * @brief Write to selected DS28E17's I2C with Stop.
        * Poll until I2C write complete and receive status info.
        *
        * @details Output on I2C: S, Address + Write, Write Data [1-255], P
        *
        * On Entry:
        * @param[in]  I2C_addr -
        * Writes I2C address. The least significant bit of the I2C
        * address is automatically cleared by the command.
        *
        * @param[in] length
        * The number of data bytes to be written ranging from 01h to FFh.
        * A value of zero will assert the Error Detected pin (ED).
        *
        * @param[in] *data
        * User defines write data ranging from 1-255 bytes.
        *
        * On Exit:
        * @param[out] status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] wr_status
        * Indicates which write byte NACK’d. A value of 00h indicates
        * all bytes were acknowledged by the slave.
        * A non-zero value indicates the byte number that NACK’d.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeDataWithStop(uint8_t I2C_addr, uint8_t length,
                                        uint8_t *data, uint8_t &status,
                                        uint8_t &wr_status);

        /**********************************************************//**
        * @brief Write to selected DS28E17's I2C No Stop.
        * Poll until I2C write complete and receive status info.
        *
        * @details Output on I2C: S, Address + Write, Write Data [1-255]
        *
        * On Entry:
        * @param[in]  I2C_addr
        * Writes I2C address. The least significant bit of the I2C address
        * is automatically cleared by the command.
        *
        * @param[in]  length
        * The number of data bytes to be written ranging from 01h to FFh.
        * A value of zero will assert the Error Detected pin (ED).
        *
        * @param[in]  *data
        * User defines write data ranging from 1-255 bytes.
        *
        * On Exit:
        * @param[out]  status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] wr_status
        * Indicates which write byte NACK’d. A value of 00h indicates
        * all bytes were acknowledged by the slave.
        * A non-zero value indicates the byte number that NACK’d.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeDataNoStop(uint8_t I2C_addr, uint8_t length,
                                      uint8_t *data, uint8_t &status,
                                      uint8_t &wr_status);


        /**********************************************************//**
        * @brief Write to selected DS28E17's I2C with Data only.
        * Poll until I2C write complete and receive status info.
        *
        * @details Output on I2C: Write Data [1-255]
        *
        * On Entry:
        * @param[in]  length
        * The number of data bytes to be written ranging from 01h to FFh.
        * A value of zero will assert the Error Detected pin (ED).
        *
        * @param[in]  *data
        * User defines write data ranging from 1-255 bytes.
        *
        * On Exit:
        * @param[out]  status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] wr_status
        * Indicates which write byte NACK’d. A value of 00h indicates all bytes were acknowledged by the slave.
        * A non-zero value indicates the byte number that NACK’d.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeDataOnly(uint8_t length, uint8_t *data,
                                    uint8_t &status, uint8_t &wr_status);


        /**********************************************************//**
        * @brief Write to selected DS28E17's I2C with Stop.
        * Poll until I2C write complete and receive status info.
        *
        * @details Output on I2C: Write Data [1-255], P
        *
        * On Entry:
        * @param[in]  length
        * The number of data bytes to be written ranging from 01h to FFh.
        * A value of zero will assert the Error Detected pin (ED).
        *
        * @param[in]  *data
        * User defines write data ranging from 1-255 bytes.
        *
        * On Exit:
        * @param[out]  status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] wr_status
        * Indicates which write byte NACK’d. A value of 00h indicates all bytes were acknowledged by the slave.
        * A non-zero value indicates the byte number that NACK’d.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeDataOnlyWithStop(uint8_t length, uint8_t *data,
                                            uint8_t &status, uint8_t &wr_status);


        /**********************************************************//**
        * @brief Write to selected DS28E17's I2C with Stop and poll until I2C write complete
        * receive status info, and read data with a stop at the end.
        *
        * @details Output on I2C:
        * S, Slave Address + Write, Write Data [1-255],
        * Sr, Address + Read, Read Data [1-255], P (NACK last read byte)
        *
        * On Entry:
        * @param[in]  I2C_addr
        * Writes I2C address. The least significant bit of the I2C address
        * is automatically cleared by the command.
        *
        * @param[in]  length
        * The number of data bytes to be written ranging from 01h to FFh.
        * A value of zero will assert the Error Detected pin (ED).
        *
        * @param[in]  *data
        * User defines write data ranging from 1-255 bytes.
        *
        * @param[in] nu_bytes_read
        * Number of I2C bytes to read. A value of zero will assert the Error Detected pin (ED).
        *
        * On Exit:
        * @param[out]  status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] wr_status
        * Indicates which write byte NACK’d. A value of 00h indicates all bytes were acknowledged by the slave.
        * A non-zero value indicates the byte number that NACK’d.
        *
        * @param[out] *read_data
        * Array of read data received from I2C.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeReadDataWithStop(uint8_t I2C_addr, uint8_t length,
                                            uint8_t *data, uint8_t nu_bytes_read,
                                            uint8_t &status, uint8_t &wr_status,
                                            uint8_t *read_data);


        /**********************************************************//**
        * @brief Selected DS28E17's and send I2C address and poll until
        * I2C read address complete, receive status info, and read data
        * with a stop at the end.
        *
        * @details Output on I2C:
        * S, Slave Address + Read, Read Data [1-255], P (NACK last read byte)
        *
        * On Entry:
        * @param[in]  I2C_addr
        * Defines the 7-bit I2C address to be written. 
        * The least significant bit must be set, indicating an I2C read.
        *
        * On Exit:
        * @param[out] nu_bytes_read
        * Number of I2C bytes to read. A value of zero will assert the Error Detected pin (ED).
        *
        * @param[out]  status
        * Detects the condition of the Start (bit3), N/A (bit2),
        * Address(bit1) and CRC16(bit0) bits.
        * b3;0=No Err|1=I2CStart prev-not issued,
        * b2;0=No Err|1=I2C N/A Err,
        * b1;0=No Err|1=Addr Err,
        * b0;0=Valid CRC16|1=Invalid CRC16
        *
        * @param[out] *read_data
        * Array of read data received from I2C.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult readDataWithStop(uint8_t I2C_addr, uint8_t nu_bytes_read,
                                       uint8_t &status, uint8_t *read_data);


        /**********************************************************//**
        * @brief Write to Configuration Register of DS28E17.
        *
        * @details
        *
        * On Entry:
        * @param[in] data
        * sent to configuration register
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeConfigReg(uint8_t data);


        /**********************************************************//**
        * @brief Read the Configuration Register of DS28E17.
        *
        * @details
        *
        * On Exit:
        * @param[out] config - contents of configuration register
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult readConfigReg(uint8_t & config);


        /**********************************************************//**
        * @brief The Enable Sleep Mode command puts the device into a low current mode.
        * All 1-Wire communication is ignored until woken up. Immediately after
        * the command, the device monitors the WAKEUP input pin and
        * exits sleep mode on a rising edge.
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult enableSleepMode();


        /**********************************************************//**
        * @brief Read the Device Revision of DS28E17. The revision value
        *  should never be zero.  The upper nibble is the major revision
        *  and the lower nibble is the minor revision.
        *
        * On Exit:
        * @param[out] rev - device revision
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult readDeviceRevision(uint8_t & rev);

    private:
        static const size_t pollLimit = 10000;

        CmdResult sendPacket(const uint8_t * data, uint8_t data_length,
                              uint8_t & status, uint8_t & wr_status);

        //overloaded function for I2C read only command 
        CmdResult sendPacket(const uint8_t * data, uint8_t data_length,
                              uint8_t & status);
    };
}

#endif

