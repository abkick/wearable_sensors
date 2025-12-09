/// @file RomCommands.h
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

#ifndef OneWire_RomCommands
#define OneWire_RomCommands

#include <stdint.h>
#include "RomId/RomId.h"
#include "Masters/OneWireMaster.h"

namespace OneWire
{
    /// Procedural 1-Wire ROM Commands for enumerating and selecting devices.
    namespace RomCommands
    {
        /// State used by all ROM ID search functions.
        struct SearchState
        {
            RomId romId;
            uint8_t last_discrepancy;
            uint8_t last_family_discrepancy;
            bool last_device_flag;

            /// Reset to the search state to start at the beginning.
            void reset();
            
            /// Setup the search to find the device type 'family_code'
            /// on the next call to OWNext() if it is present.
            void findFamily(uint8_t familyCode);

            /// Setup the search to skip the current device type on the
            /// next call to OWNext().
            void skipCurrentFamily();

            SearchState() { reset(); }
        };
        
        ///Find the 'first' devices on the 1-Wire bus.
        OneWireMaster::CmdResult OWFirst(OneWireMaster & master, SearchState & searchState);

        /// Find the 'next' devices on the 1-Wire bus.
        OneWireMaster::CmdResult OWNext(OneWireMaster & master, SearchState & searchState);

        /// Verify that the device with the specified ROM ID is present.
        OneWireMaster::CmdResult OWVerify(OneWireMaster & master, const RomId & romId);

        /// Use Read ROM command to read ROM ID from device on bus.
        /// @note Only use this command with a single drop bus, data
        ///       collisions will occur if more than 1 device on bus.
        /// @param[out] romId ROM ID read from device.
        OneWireMaster::CmdResult OWReadRom(OneWireMaster & master, RomId & romId);

        /// Issue Skip ROM command on bus.
        /// @note Only use this command with a single drop bus, data
        ///       collisions will occur if more than 1 device on bus.
        OneWireMaster::CmdResult OWSkipRom(OneWireMaster & master);

        /// Use the Match ROM command to select the device by its known ID.
        /// @note This command causes all devices supporting Overdrive
        ///       mode to switch to Overdrive timing.
        /// @param[in] romId ROM ID of device to select.
        OneWireMaster::CmdResult OWMatchRom(OneWireMaster & master, const RomId & romId);

        /// Issue Overdrive Skip ROM command on bus.
        /// @note This command causes all devices supporting Overdrive
        ///       mode to switch to Overdrive timing.
        /// @note Only use this command with a single drop bus, data
        ///       collisions will occur if more than 1 device on bus.
        OneWireMaster::CmdResult OWOverdriveSkipRom(OneWireMaster & master);

        /// Use the Overdrive Match ROM command to select the device by its known ID.
        /// @param[in] romId ROM ID of device to select.
        OneWireMaster::CmdResult OWOverdriveMatchRom(OneWireMaster & master, const RomId & romId);

        /// Perform a Resume ROM command on bus.
        /// @details Resumes communication with the last device selected
        ///          though a Match ROM or Search ROM operation.
        OneWireMaster::CmdResult OWResume(OneWireMaster & master);

        /// Find device on the 1-Wire bus.
        /// @details This command uses the Search ROM command to enumerate all 1-Wire devices in sequence.
        ///          Begin with a new search state and continue using the same search state until the last
        ///          device flag is set which indicates that all devices have been discovered.
        OneWireMaster::CmdResult OWSearch(OneWireMaster & master, SearchState & searchState);
        
        /// Find Alarming Devices on 1-Wire bus.
        /// @details This command uses the Alarm Search ROM command to enumerate all 1-Wire devices with active alarms
        ///          Begin with a new search state and continue using the same search state until the last
        ///          device flag is set which indicates that all devices have been discovered.          
        OneWireMaster::CmdResult OWAlarmSearch(OneWireMaster & master, SearchState & searchState);
        
        /// Find the first Alarming device on the 1-Wire bus.
        /// @details This command resets the SearchState and uses the Alarm Search command to enumerate the first actively alarming device on the bus, in numerical order by ROM ID.
        OneWireMaster::CmdResult OWFirstAlarm(OneWireMaster & master, SearchState & searchState);
        
        /// Find the next Alarming Device on the 1-Wire bus.
        /// @details This command  uses the Alarm Search command to enumerate the next actively alarming device on the bus, which is determined by the current searchState.
        OneWireMaster::CmdResult OWNextAlarm(OneWireMaster & master, SearchState & searchState);
        
        /// Helper function for the above search functions
        OneWireMaster::CmdResult OWSearchAll(OneWireMaster & master, SearchState & searchState, bool alarmSearch);

    }
}

#endif
