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

#ifndef OneWire_Switches_DS2413
#define OneWire_Switches_DS2413

#include <stdint.h>
#include "Slaves/OneWireSlave.h"

namespace OneWire
{
    /**
    * @brief DS2413 1-Wire Dual Channel Addressable Switch
    *
    * @details The DS2413 is a dual-channel programmable I/O 1-Wire® 
    * chip. The PIO outputs are configured as open-drain and provide up
    * to 20mA continuous sink capability and off-state operating voltage
    * up to 28V. Control and sensing of the PIO pins is performed with 
    * a dedicated device-level command protocol. To provide a high level 
    * of fault tolerance in the end application, the 1-Wire I/O and PIO 
    * pins are all capable of withstanding continuous application of 
    * voltages up to 28V max. Communication and operation of the DS2413 
    * is performed with the single contact Maxim 1-Wire serial interface.
    */
    class DS2413 : public OneWireSlave
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
        * @brief DS2413 constructor
        *
        * @details
        *
        * On Entry:
        *     @param[in] owm - reference to 1-wire master
        *
        * On Exit:
        *
        * @return
        **************************************************************/
        DS2413(RandomAccessRomIterator &selector);

        /**********************************************************//**
        * @brief pioAccessReadChA()
        *
        * @details reads state of pio
        *
        *
        * On Exit:
        *     @param[out] val - lsb represents the state of the pio
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult pioAccessReadChA(uint8_t & val);

        /**********************************************************//**
        * @brief pioAccessReadChB()
        *
        * @details reads state of pio
        *
        *
        * On Exit:
        *     @param[out] val - lsb represents the state of the pio
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult pioAccessReadChB(uint8_t & val);

        /**********************************************************//**
        * @brief pioAccessWriteChA()
        *
        * @details writes to pio
        *
        * On Entry:
        *    @param[in] val - lsb sets state of pio
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult pioAccessWriteChA(uint8_t val);

        /**********************************************************//**
        * @brief pioAccessWriteChB()
        *
        * @details writes to pio
        *
        * On Entry:
        *    @param[in] val - lsb sets state of pio
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult pioAccessWriteChB(uint8_t val);

        /**********************************************************//**
        * @brief pioAccessWriteChAB()
        *
        * @details writes to pio
        *
        * On Entry:
        *    @param[in] val - Bits 1:0 set PIOB and PIOB respectively
        *
        * @return CmdResult - result of operation
        **************************************************************/
        CmdResult pioAccessWriteChAB(uint8_t val);

    private:

        CmdResult pioAccessRead(uint8_t & val);

        CmdResult pioAccessWrite(uint8_t val);
    };
}

#endif
