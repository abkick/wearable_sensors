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

#ifdef TARGET_MAX32600

#include "Masters/TARGET_Maxim/TARGET_MAX32600/OwGpio/OwGpio.h"
#include "Masters/TARGET_Maxim/TARGET_MAX32600/OwGpio/owlink.h"
#include "gpio_regs.h"
#include "clkman_regs.h"


using OneWire::OneWireMaster;
using OneWire::OwGpio;

static const OwTiming stdTiming = {
                                      560, // tRSTL
                                      68, // tMSP
                                      64, // tW0L
                                      8, // tW1L
                                      12, // tMSR
                                      70, // tSLOT
                                  };

static const OwTiming odTiming = {
                                      56, // tRSTL
                                      8, // tMSP
                                      8, // tW0L
                                      1, // tW1L
                                      1, // tMSR
                                      10, // tSLOT
                                 };

OwGpio::OwGpio(PinName owGpio, PinName extSpu, bool extSpuActiveHigh)
    : owPort(PINNAME_TO_PORT(owGpio)), owPin(PINNAME_TO_PIN(owGpio)), owSpeed(StandardSpeed),
        extSpu(extSpu), extSpuActiveHigh(extSpuActiveHigh)
{

}

OneWireMaster::CmdResult OwGpio::OWInitMaster()
{
    if (MXC_CLKMAN->clk_ctrl_1_gpio == MXC_E_CLKMAN_CLK_SCALE_DISABLED)
    {
        MXC_CLKMAN->clk_ctrl_1_gpio = MXC_E_CLKMAN_CLK_SCALE_ENABLED;
    }

    /* Set function */
    MXC_GPIO->func_sel[owPort] &= ~(0xF << (4 * owPin));

    /* Normal input is always enabled */
    MXC_GPIO->in_mode[owPort] &= ~(0xF << (4 * owPin));

    writeOwGpioHigh();
    OWSetLevel(NormalLevel);

    return OneWireMaster::Success;
}

OneWireMaster::CmdResult OwGpio::OWReset()
{
    const OwTiming & curTiming(owSpeed == OverdriveSpeed ? odTiming : stdTiming);
    uint16_t tREC = curTiming.tRSTL - curTiming.tMSP; // tSLOT = 2 *tRSTL

    __disable_irq(); // Enter critical section
    
    writeOwGpioLow(); // Pull low
    ow_usdelay(curTiming.tRSTL); // Wait specified time for reset pulse
    writeOwGpioHigh(); // Let go of pin
    ow_usdelay(curTiming.tMSP); // Wait specified time for master sample
    uint8_t pd_pulse = readOwGpio(); // Get sample
    ow_usdelay(tREC); // Wait for slot time to finish including recovery
    
    __enable_irq(); // Exit critical section

    return((pd_pulse == 0) ? OneWireMaster::Success : OneWireMaster::OperationFailure);
}

OneWireMaster::CmdResult OwGpio::OWTouchBitSetLevel(uint8_t & sendRecvBit, OWLevel afterLevel)
{
    __disable_irq(); // Enter critical section

    ow_bit(&sendRecvBit, &MXC_GPIO->in_val[owPort], &MXC_GPIO->out_val[owPort], (1 << owPin),
            ((owSpeed == OverdriveSpeed) ? &odTiming : &stdTiming));
    OWSetLevel(afterLevel);

    __enable_irq(); // Exit critical section

    return OneWireMaster::Success;
}

OneWireMaster::CmdResult OwGpio::OWWriteByteSetLevel(uint8_t sendByte, OWLevel afterLevel)
{
    OneWireMaster::CmdResult result;

    for (uint8_t idx = 0; idx < 8; idx++)
    {
        result = OneWireMaster::OWWriteBit(0x01 & (sendByte >> idx));
        if (result != OneWireMaster::Success)
        {
            break;
        }
    }

    if (result == OneWireMaster::Success)
    {
        OWSetLevel(afterLevel);
    }

    return result;
}

OneWireMaster::CmdResult OwGpio::OWReadByteSetLevel(uint8_t & recvByte, OWLevel afterLevel)
{
    OneWireMaster::CmdResult result;
    uint8_t recvBit;
    recvByte = 0;

    for (unsigned int idx = 0; idx < 8; idx++)
    {
        result = OneWireMaster::OWReadBit(recvBit);
        if (result != OneWireMaster::Success)
        {
            break;
        }
        //else
        recvByte |= ((0x01 & recvBit) << idx);
    }

    if (result == OneWireMaster::Success)
    {
        OWSetLevel(afterLevel);
    }

    return result;
}

OneWireMaster::CmdResult OwGpio::OWSetSpeed(OWSpeed newSpeed)
{
    owSpeed = newSpeed;
    return OneWireMaster::Success;
}

OneWireMaster::CmdResult OwGpio::OWSetLevel(OWLevel newLevel)
{    
    switch (newLevel)
    {
    case NormalLevel:
    default:
        setOwGpioMode(MXC_V_GPIO_OUT_MODE_OPEN_DRAIN);
        if (extSpu.is_connected())
        {
            extSpu = !extSpuActiveHigh;
        }
        break;

    case StrongLevel:
        if (extSpu.is_connected())
        {
            extSpu = extSpuActiveHigh;
        }
        else
        {
            setOwGpioMode(MXC_V_GPIO_OUT_MODE_NORMAL_DRIVE);
        }
        break;
    }

    return OneWireMaster::Success;
}

inline void OwGpio::writeOwGpioLow()
{
    MXC_GPIO->out_val[owPort] &= ~(1 << owPin);
}

inline void OwGpio::writeOwGpioHigh()
{
    MXC_GPIO->out_val[owPort] |= (1 << owPin);
}

inline bool OwGpio::readOwGpio()
{
    return ((MXC_GPIO->in_val[owPort] & (1 << owPin)) >> owPin);
}

inline void OwGpio::setOwGpioMode(unsigned int mode)
{
    //read port out_mode
    uint32_t ow_out_mode = MXC_GPIO->out_mode[owPort];
    //clear the mode for ow_pin
    ow_out_mode &= ~(0xF << (owPin * 4));
    //write ow_pin mode and original data back
    MXC_GPIO->out_mode[owPort] = (ow_out_mode | (mode << (owPin * 4)));
}

#endif /* TARGET_MAX32600 */
