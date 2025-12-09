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

#ifndef OneWire_OneWireSlave
#define OneWire_OneWireSlave

#include <stddef.h>
#include "RomId/RomId.h"
#include "RomId/RomIterator.h"

namespace OneWire
{
    class OneWireMaster;
    
    /// Base class for all 1-Wire Slaves.
    class OneWireSlave
    {
    public:
        enum CmdResult
        {
            Success,
            CommunicationError,
            CrcError,
            TimeoutError,
            OperationFailure
        };

        /// @{
        /// 1-Wire ROM ID for this slave device.
        RomId romId() const { return m_romId; }
        void setRomId(const RomId & romId) { m_romId = romId; }
        /// @}
        
    private:
        RomId m_romId;
        RandomAccessRomIterator & m_selector;
        
    protected:
        /// @param selector Provides 1-Wire ROM selection and bus access. 
        OneWireSlave(RandomAccessRomIterator & selector) : m_selector(selector) { }
        
        ~OneWireSlave() { }
        
        /// Select this slave device by ROM ID.
        OneWireMaster::CmdResult selectDevice() const { return m_selector.selectDevice(m_romId); }
        
        /// The 1-Wire master for this slave device.
        OneWireMaster & master() const { return m_selector.master(); }
    };
}

#endif
