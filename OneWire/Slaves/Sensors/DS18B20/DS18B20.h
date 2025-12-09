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

#ifndef OneWire_Slaves_Sensors_DS18B20
#define OneWire_Slaves_Sensors_DS18B20

#include "Slaves/OneWireSlave.h"

namespace OneWire
{
    class OneWireMaster;
    
    /**
    * @brief DS18B20 Programmable Resolution 1-Wire Digital Thermometer
    *
    * @details The DS18B20 digital thermometer provides 9-bit to 12-bit 
    * Celsius temperature measurements and has an alarm function with 
    * nonvolatile user-programmable upper and lower trigger points. The 
    * DS18B20 communicates over a 1-Wire bus that by definition requires 
    * only one data line (and ground) for communication with a central 
    * microprocessor. In addition, the DS18B20 can derive power directly 
    * from the data line ("parasite power"), eliminating the need for an 
    * external power supply.
    *
    * @code
    * @endcode
    */
    class DS18B20 : public OneWireSlave
    {
    public:
        
        ///Available resolutions of the DS18B20
        enum Resolution
        {
            NineBit = 0x1F,
            TenBit = 0x3F,
            ElevenBit = 0x5F,
            TwelveBit = 0x7F
        };

        /**********************************************************//**
        * @brief DS18B20 constructor
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
        DS18B20(RandomAccessRomIterator &selector);
        
        
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
        * @param[in] tl - 8-bit lower temperature threshold, LSB 
        * indicates sign 
        * @param[in] res - Resolution of the DS18B20
        *
        * On Exit:
        * @param[out]
        *
        * @return CmdResult - result of operation
        **************************************************************/
        OneWireSlave::CmdResult writeScratchPad(uint8_t th, uint8_t tl, Resolution res);
        
        
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
        OneWireSlave::CmdResult readScratchPad(uint8_t * scratchPadBuff);
        
        /**********************************************************//**
        * @brief Copy Scratchpad Command
        *
        * @details This command copies from the scratchpad into the 
        * EEPROM of the DS18B20, storing the temperature trigger bytes 
        * and resolution in nonvolatile memory.
        *
        * On Entry:
        * @param[in] 
        *
        * On Exit:
        * @param[out]
        *
        * @return CmdResult - result of operation
        **************************************************************/
        OneWireSlave::CmdResult copyScratchPad( void );
        
        
        /**********************************************************//**
        * @brief Read Power Supply command
        *
        * @details This command determines if the DS18B20 is parasite
        * powered or has a local supply
        *
        * On Entry:
        * @param[in] 
        *
        * On Exit:
        * @param[out] localPower - Will be False on exit if the DS18B20
        * is parasite powered
        *
        * @return CmdResult - result of operation
        **************************************************************/
        OneWireSlave::CmdResult readPowerSupply(bool & localPower);
        
        
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
        OneWireSlave::CmdResult convertTemperature(float & temp);
        
        
        /**********************************************************//**
        * @brief Recall Command
        *
        * @details This command recalls the temperature trigger values 
        * and resolution stored in EEPROM to the scratchpad
        *
        * On Entry:
        * @param[in]
        *
        * On Exit:
        * @param[out] 
        *
        * @return CmdResult - result of operation
        **************************************************************/
        OneWireSlave::CmdResult recallEEPROM( void );

    };
}

#endif /* OneWire_Slaves_Sensors_DS18B20 */
