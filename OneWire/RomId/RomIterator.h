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

#ifndef OneWire_RomIterator
#define OneWire_RomIterator

#include <stdint.h>
#include "RomId/RomCommands.h"

namespace OneWire
{
    class OneWireMaster;
    
    /// Controls selection of 1-Wire devices on the bus through ROM commands.
    class RomIterator
    {
    private:
        OneWireMaster & owMaster;
        
    protected:
        /// @param master 1-Wire master to use to issue ROM commands.
        RomIterator(OneWireMaster & master) : owMaster(master) { }
        
    public:
        virtual ~RomIterator() { }
        
        /// The 1-Wire master used to issue ROM commands.
        OneWireMaster & master() const { return owMaster; }
    };
    
    /// Iterates through all 1-Wire devices in a sequential first to last order.
    class ForwardRomIterator : public RomIterator
    {
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        ForwardRomIterator(OneWireMaster & master) : RomIterator(master) { }
        
        /// Indicates that current device is the last.
        virtual bool lastDevice() const = 0;
        
        /// Select the first device in the sequence.
        virtual OneWireMaster::CmdResult selectFirstDevice() = 0;
        
        /// Select the next device in the sequence.
        virtual OneWireMaster::CmdResult selectNextDevice() = 0;
        
        /// Reselect the current device for an additional operation.
        virtual OneWireMaster::CmdResult reselectCurrentDevice() = 0;
    };
    
    /// Iterates through all 1-Wire devices sequentially using the search procedure.
    class ForwardSearchRomIterator : public ForwardRomIterator
    {
    protected:
        RomCommands::SearchState searchState;
        
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        ForwardSearchRomIterator(OneWireMaster & master) : ForwardRomIterator(master) { }
        
        /// ROM ID of the currently selected device.
        const RomId & selectedDevice() const { return searchState.romId; }
        
        virtual bool lastDevice() const;
        virtual OneWireMaster::CmdResult selectFirstDevice();
        virtual OneWireMaster::CmdResult selectNextDevice();
        virtual OneWireMaster::CmdResult reselectCurrentDevice();
        
        /// Select the first device in the sequence beginning with the given family.
        /// @param familyCode Family code to select.
        OneWireMaster::CmdResult selectFirstDeviceInFamily(uint8_t familyCode);
        
        /// Select the first device in the next sequential family skipping all remaining devices
        /// in the current family.
        OneWireMaster::CmdResult selectNextFamilyDevice();
    };
    
    /// Iterates though 1-Wire devices on the bus using random selection by ROM ID.
    class RandomAccessRomIterator : public RomIterator
    {
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        RandomAccessRomIterator(OneWireMaster & master) : RomIterator(master) { }
        
        /// Select the device with the given ROM ID.
        virtual OneWireMaster::CmdResult selectDevice(const RomId & romId) = 0;
    };
    
    /// Iterator for a singledrop 1-Wire bus.
    class SingledropRomIterator : public RandomAccessRomIterator
    {
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        SingledropRomIterator(OneWireMaster & master) : RandomAccessRomIterator(master) { }
        
        /// Select the one and only device.
        OneWireMaster::CmdResult selectDevice() { return RomCommands::OWSkipRom(master()); }
        virtual OneWireMaster::CmdResult selectDevice(const RomId & romId);
    };
    
    /// Iterator for a multidrop 1-Wire bus.
    class MultidropRomIterator : public RandomAccessRomIterator
    {        
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        MultidropRomIterator(OneWireMaster & master) : RandomAccessRomIterator(master) { }
        
        virtual OneWireMaster::CmdResult selectDevice(const RomId & romId);
    };
    
    /// Iterator for a multidrop 1-Wire bus where slaves support the Resume ROM command.
    class MultidropRomIteratorWithResume : public RandomAccessRomIterator
    {
    private:
        RomId lastRom;
        
    public:
        /// @param master 1-Wire master to use to issue ROM commands.
        MultidropRomIteratorWithResume(OneWireMaster & master)
            : RandomAccessRomIterator(master), lastRom() { }
        
        virtual OneWireMaster::CmdResult selectDevice(const RomId & romId);
    };
}

#endif
