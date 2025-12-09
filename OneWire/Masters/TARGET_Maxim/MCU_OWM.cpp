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

#if defined(TARGET_MAX32620) || defined(TARGET_MAX32625) || defined(TARGET_MAX32630)

#include "MCU_OWM.h"

using namespace OneWire;

MCU_OWM::MCU_OWM(bool extWeakPup, bool extStrongPup, bool long_line)
{
    m_sysCfgOwm.clk_scale = CLKMAN_SCALE_AUTO;
     
    if(extStrongPup)
    {
        m_owmCfg.ext_pu_mode = OWM_EXT_PU_ACT_LOW;
        ioman_cfg_t cfg = IOMAN_OWM(1, 1);
        m_sysCfgOwm.io_cfg = cfg;
    }
    else
    {
        m_owmCfg.ext_pu_mode = OWM_EXT_PU_UNUSED;
        ioman_cfg_t cfg = IOMAN_OWM(1, 0);
        m_sysCfgOwm.io_cfg = cfg;
    }
    
    if(extWeakPup)
    {
        m_owmCfg.int_pu_en = 0;
    }
    else
    {
        m_owmCfg.int_pu_en = 1;
    }
    if(long_line)
    {
        m_owmCfg.long_line_mode = 1;
    }
    else
    {
        m_owmCfg.long_line_mode = 0;
    }
    //m_owmCfg.long_line_mode = 0;
    m_owmCfg.overdrive_spec = OWM_OVERDRIVE_UNUSED;  
}

//**********************************************************************
bool MCU_OWM::extStrongPup() const
{
    return m_owmCfg.ext_pu_mode == OWM_EXT_PU_ACT_LOW;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWInitMaster()
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    if(OWM_Init(MXC_OWM, &m_owmCfg, &m_sysCfgOwm) == E_NO_ERROR)
    {
        result = OneWireMaster::Success;
    }
    
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWReset()
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    if(OWM_Reset(MXC_OWM))
    {
        result = OneWireMaster::Success;
    }
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWTouchBitSetLevel(uint8_t & sendRecvBit, OWLevel afterLevel)
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    sendRecvBit = OWM_TouchBit(MXC_OWM, sendRecvBit);
    
    if(extStrongPup())
    {
        result = OWSetLevel(afterLevel);
    }
    else
    {
        result = OneWireMaster::Success;
    }
    
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWWriteByteSetLevel(uint8_t sendByte, OWLevel afterLevel)
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    if(OWM_WriteByte(MXC_OWM, sendByte) == E_NO_ERROR)
    {
        if(extStrongPup())
        {
            result = OWSetLevel(afterLevel);
        }
        else
        {
            result = OneWireMaster::Success;
        }
    }
    
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWReadByteSetLevel(uint8_t & recvByte, OWLevel afterLevel)
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    recvByte = OWM_ReadByte(MXC_OWM);
    
    if(extStrongPup())
    {
        result = OWSetLevel(afterLevel);
    }
    else
    {
        result = OneWireMaster::Success;
    }
    
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWSetSpeed(OWSpeed newSpeed)
{
    OneWireMaster::CmdResult result = OneWireMaster::Success;
    
    if(newSpeed == OneWireMaster::OverdriveSpeed)
    {
        OWM_SetOverdrive(MXC_OWM, 1);
    }
    else if(newSpeed == OneWireMaster::StandardSpeed)
    {
        OWM_SetOverdrive(MXC_OWM, 0);
    }
    else
    {
        result = OneWireMaster::OperationFailure;
    }
    
    return result;
}

//**********************************************************************
OneWireMaster::CmdResult MCU_OWM::OWSetLevel(OWLevel newLevel)
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    
    if(extStrongPup())
    {
        if(newLevel == OneWireMaster::StrongLevel)
        {
            OWM_SetExtPullup(MXC_OWM, 1);
        }
        else
        {
            OWM_SetExtPullup(MXC_OWM, 0);
        }
        
        result = OneWireMaster::Success;
    }
    
    return result;
}

#endif /* TARGET_MAX326xx */

