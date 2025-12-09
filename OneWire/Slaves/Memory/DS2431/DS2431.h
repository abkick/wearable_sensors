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

#ifndef OneWire_Slaves_Memory_DS2431
#define OneWire_Slaves_Memory_DS2431

#include "Slaves/OneWireSlave.h"

namespace OneWire
{    
    /**
    * @brief DS2431 1024-Bit 1-Wire EEPROM
    * @details The DS2431 is a 1024-bit, 1-Wire® EEPROM chip organized 
    * as four memory pages of 256 bits each. Data is written to an 8-byte 
    * scratchpad, verified, and then copied to the EEPROM memory. As a 
    * special feature, the four memory pages can individually be write 
    * protected or put in EPROM-emulation mode, where bits can only be 
    * changed from a 1 to a 0 state. The DS2431 communicates over the 
    * single-conductor 1-Wire bus. The communication follows the standard 
    * 1-Wire protocol. Each device has its own unalterable and unique 
    * 64-bit ROM registration number that is factory lasered into the chip. 
    * The registration number is used to address the device in a multidrop, 
    * 1-Wire net environment.
    */
    class DS2431 : public OneWireSlave
    {
    public:
        typedef array<uint8_t, 8> Scratchpad;
        typedef uint16_t Address;
    
        /**********************************************************//**
        * @brief DS2431 constructor
        *
        * @details Instantiate a DS2431 object that encapsulates the 
        * 1-Wire master and ROM commands for selecting the device via 
        * the RandomAccessRomIterator sub-class passed as an argument 
        * to the constructor.  
        *
        * This allows the user to focus on the use of the DS2431 in
        * their application vs. the low level details of the 1-Wire 
        * protocol.
        *
        * On Entry:
        * @param[in] selector - reference to RandomAccessRomIterator
        * sub-class; i.e. SingledropRomIterator, MultidropRomIterator, etc.
        * See RomId/RomIterator.h
        *
        * On Exit:
        *
        * @return
        **************************************************************/
        DS2431(RandomAccessRomIterator & selector);
        
        /**********************************************************//**
        * @brief writeMemory
        *
        * @details Writes data to EEPROM.  Wraps up writeScratchPad,
        * readScratchPad and copyScratchPad into single function.
        * 
        * On Entry:
        * @param[in] targetAddress - EEPROM memory address to start 
        * writing at.
        *
        * @param[in] data - Pointer to memory holding data.
        *
        * @param[in] numBytes - Number of bytes to write.
        *
        * On Exit: 
        *
        * @return Result of operation
        **************************************************************/
        OneWireSlave::CmdResult writeMemory(Address targetAddress, const Scratchpad & data);
        
        /**********************************************************//**
        * @brief readMemory
        *
        * @details Reads block of data from EEPROM memory.
        *
        * On Entry:
        * @param[in] targetAddress - EEPROM memory address to start.
        * reading from
        *
        * @param[out] data - Pointer to memory for storing data.
        *
        * @param[in] numBytes - Number of bytes to read.
        *
        * On Exit:
        *
        * @return Result of operation
        **************************************************************/
        OneWireSlave::CmdResult readMemory(Address targetAddress, uint8_t numBytes, uint8_t * data);
        
    private:    
        /**********************************************************//**
        * @brief writeScratchpad
        *
        * @details Writes 8 bytes to the scratchpad.
        *
        * On Entry:
        * @param[in] targetAddress - EEPROM memory address that this data 
        * will be copied to.  Must be on row boundary.
        *
        * @param[in] data - reference to bounded array type Scratchpad.
        *
        * On Exit:
        *
        * @return Result of operation
        **************************************************************/
        OneWireSlave::CmdResult writeScratchpad(Address targetAddress, const Scratchpad & data);
        
        /**********************************************************//**
        * @brief readScratchpad
        *
        * @details Reads contents of scratchpad.
        *
        * On Entry:
        * @param[out] data - reference to bounded array type Scratchpad.
        *
        * On Exit:
        *
        * @return Result of operation
        **************************************************************/
        OneWireSlave::CmdResult readScratchpad(Scratchpad & data, uint8_t & esByte);
        
        /**********************************************************//**
        * @brief copyScratchpad
        *
        * @details Copies contents of sractshpad to EEPROM.
        *
        * On Entry:
        * @param[in] targetAddress - EEPROM memory address that this data 
        * will be copied to.  Must be on row boundary.
        *
        * @param[in] esByte - Returned from reading scratchpad.
        *
        * On Exit:
        *
        * @return Result of operation
        **************************************************************/
        OneWireSlave::CmdResult copyScratchpad(Address targetAddress, uint8_t esByte);
    };
}

#endif /*OneWire_Slaves_Memory_DS2431*/