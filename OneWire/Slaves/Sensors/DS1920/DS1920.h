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

#ifndef OneWire_Temperature_DS1920
#define OneWire_Temperature_DS1920

#include "Slaves/OneWireSlave.h"

namespace OneWire
{
    class OneWireMaster;
    
    /**
    * @brief DS1920 1-wire temperature i-button
    *
    * @details The iButton® temperature logger (DS1920) provides 
    * direct-to-digital 9-bit temperature readings over a range of 
    * -55°C to +100°C in 0.5° increments. The iButton communicates with
    * a processor using the 1-Wire® protocol through a hardware port 
    * interface. The port interface provides both the physical link and 
    * handles the communication protocols that enable the processor to
    * access iButton resources with simple commands. Two bytes of 
    * EEPROM can be used either to set alarm triggers or for storing 
    * user data.
    *
    * @code
    * @endcode
    */
    class DS1920 : public OneWireSlave
    {
    public:
        
        ///Result of operations
        enum CmdResult
        {
            Success,
            CommsReadError,
            CommsWriteError,
            OpFailure
        };

        /**********************************************************//**
        * @brief DS1920 constructor
        *
        * @details
        *
        * On Entry:
        * @param[in] selector - Reference to RandomAccessRomiteraor 
        * object that encapsulates owm master that has access to this 
        * device and ROM function commands used to a select device
        *
        * On Exit:
        *
        * @return
        **************************************************************/
        DS1920(RandomAccessRomIterator &selector);
        
        
        /**********************************************************//**
        * @brief Write Scratchpad Command
        *
        * @details If the result of a temperature measurement is higher 
        * than TH or lower than TL, an alarm flag inside the device is 
        * set. This flag is updated with every temperature measurement. 
        * As long as the alarm flag is set, the DS1920 will respond to 
        * the alarm search command.
        *
        * On Entry:
        * @param[in] th - 8-bit upper temperature threshold, MSB 
        * indicates sign 
        * @param[in] tl - 8-bit lower temperature threshold, MSB 
        * indicates sign 
        *
        * On Exit:
        * @param[out]
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult writeScratchPad(uint8_t th, uint8_t tl);
        
        
        /**********************************************************//**
        * @brief Read Scratchpad Command
        *
        * @details This command reads the complete scratchpad.
        *
        * On Entry:
        * @param[in] scratchPadBuff - array for receiving contents of 
        * scratchpad, this buffer will be over written
        *
        * On Exit:
        * @param[out] scratchPadBuff - contents of scratchpad
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult readScratchPad(uint8_t * scratchPadBuff);
        
        /**********************************************************//**
        * @brief Copy Scratchpad Command
        *
        * @details This command copies from the scratchpad into the 
        * EEPROM of the DS1920, storing the temperature trigger bytes 
        * in nonvolatile memory.
        *
        * On Entry:
        * @param[in] 
        *
        * On Exit:
        * @param[out]
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult copyScratchPad( void );
        
        /**********************************************************//**
        * @brief Convert Temperature Command
        *
        * @details This command begins a temperature conversion. 
        *
        * On Entry:
        * @param[in]
        *
        * On Exit:
        * @param[out] temp - temperature conversion results
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult convertTemperature(float & temp);
        
        
        /**********************************************************//**
        * @brief Recall Command
        *
        * @details This command recalls the temperature trigger values 
        * stored in EEPROM to the scratchpad
        *
        * On Entry:
        * @param[in]
        *
        * On Exit:
        * @param[out] 
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult recallEEPROM( void );
        
    private:

    };
}

#endif /* OneWire_Temperature_DS1920 */
