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

#include "RomId/RomIterator.h"
#include "Masters/OneWireMaster.h"

using namespace OneWire;
using namespace OneWire::RomCommands;

bool ForwardSearchRomIterator::lastDevice() const
{
    return searchState.last_device_flag;
}

OneWireMaster::CmdResult ForwardSearchRomIterator::selectFirstDevice()
{
    return OWFirst(master(), searchState);
}

OneWireMaster::CmdResult ForwardSearchRomIterator::selectNextDevice()
{
    return OWNext(master(), searchState);
}

OneWireMaster::CmdResult ForwardSearchRomIterator::reselectCurrentDevice()
{
    return OWResume(master());
}

OneWireMaster::CmdResult ForwardSearchRomIterator::selectFirstDeviceInFamily(uint8_t familyCode)
{
    searchState.findFamily(familyCode);
    return OWNext(master(), searchState);
}

OneWireMaster::CmdResult ForwardSearchRomIterator::selectNextFamilyDevice()
{
    searchState.skipCurrentFamily();
    return OWNext(master(), searchState);
}

OneWireMaster::CmdResult SingledropRomIterator::selectDevice(const RomId &)
{
    return selectDevice();
}

OneWireMaster::CmdResult MultidropRomIterator::selectDevice(const RomId & romId)
{
    return OWMatchRom(master(), romId);
}

OneWireMaster::CmdResult MultidropRomIteratorWithResume::selectDevice(const RomId & romId)
{
    OneWireMaster::CmdResult result;
    
    if (romId == lastRom)
    {
        result = OWResume(master());
    }
    else
    {
        result = OWMatchRom(master(), romId);
        lastRom = romId;
    }
    
    return result;
}
