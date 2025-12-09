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

#include "RomId/RomCommands.h"

using namespace OneWire::crc;

namespace OneWire
{
    namespace RomCommands
    {
        enum OwRomCmd
        {
            ReadRomCmd = 0x33,
            MatchRomCmd = 0x55,
            SearchRomCmd = 0xF0,
            SkipRomCmd = 0xCC,
            ResumeCmd = 0xA5,
            OverdriveSkipRomCmd = 0x3C,
            OverdriveMatchRomCmd = 0x69,
            AlarmSearchCmd = 0xEC
        };
        
        void SearchState::reset()
        {
            last_discrepancy = 0;
            last_device_flag = false;
            last_family_discrepancy = 0;
            romId = RomId();
        }

        void SearchState::findFamily(uint8_t familyCode)
        {
            reset();
            romId.familyCode() = familyCode;
            last_discrepancy = 64;
        }

        void SearchState::skipCurrentFamily()
        {
            // set the Last discrepancy to last family discrepancy
            last_discrepancy = last_family_discrepancy;

            // clear the last family discrpepancy
            last_family_discrepancy = 0;

            // check for end of list
            if (last_discrepancy == 0)
            {
                last_device_flag = true;
            }
        }

        OneWireMaster::CmdResult OWFirst(OneWireMaster & master, SearchState & searchState)
        {
            // Reset and begin a new search
            searchState.reset();
            return OWSearch(master, searchState);
        }

        OneWireMaster::CmdResult OWNext(OneWireMaster & master, SearchState & searchState)
        {
            // Continue the previous search
            return OWSearch(master, searchState);
        }

        OneWireMaster::CmdResult OWVerify(OneWireMaster & master, const RomId & romId)
        {
            OneWireMaster::CmdResult result;
            SearchState searchState;

            searchState.romId = romId;

            // set search to find the same device
            searchState.last_discrepancy = 64;
            searchState.last_device_flag = false;

            result = OWSearch(master, searchState);
            if (result == OneWireMaster::Success)
            {
                // check if same device found
                if (romId != searchState.romId)
                {
                    result = OneWireMaster::OperationFailure;
                }
            }

            return result;
        }

        OneWireMaster::CmdResult OWReadRom(OneWireMaster & master, RomId & romId)
        {
            OneWireMaster::CmdResult result;
            RomId readId;

            result = master.OWReset();
            if (result == OneWireMaster::Success)
            {
                result = master.OWWriteByte(ReadRomCmd);
            }

            // read the ROM
            if (result == OneWireMaster::Success)
            {
                result = master.OWReadBlock(readId.buffer.data(), readId.buffer.size());
            }

            // verify CRC8
            if (result == OneWireMaster::Success)
            {
                if (readId.valid())
                {
                    romId = readId;
                }
                else
                {
                    result = OneWireMaster::OperationFailure;
                }
            }

            return result;
        }
        
        OneWireMaster::CmdResult OWSkipRom(OneWireMaster & master)
        {
            OneWireMaster::CmdResult result;

            result = master.OWReset();
            if (result == OneWireMaster::Success)
            {
                result = master.OWWriteByte(SkipRomCmd);
            }

            return result;
        }
        
        OneWireMaster::CmdResult OWMatchRom(OneWireMaster & master, const RomId & romId)
        {
            OneWireMaster::CmdResult result;

            uint8_t buf[1 + RomId::Buffer::csize];

            // use MatchROM
            result = master.OWReset();
            if (result == OneWireMaster::Success)
            {
                buf[0] = MatchRomCmd;
                std::memcpy(&buf[1], romId.buffer.data(), romId.buffer.size());
                // send command and rom
                result = master.OWWriteBlock(buf, 1 + romId.buffer.size());
            }

            return result;
        }
        
        OneWireMaster::CmdResult OWOverdriveSkipRom(OneWireMaster & master)
        {
            OneWireMaster::CmdResult result = master.OWSetSpeed(OneWireMaster::StandardSpeed);

            if (result == OneWireMaster::Success)
            {
                result = master.OWReset();
            }

            if (result == OneWireMaster::Success)
            {
                result = master.OWWriteByte(OverdriveSkipRomCmd);
            }

            if (result == OneWireMaster::Success)
            {
                result = master.OWSetSpeed(OneWireMaster::OverdriveSpeed);
            }

            return result;
        }
          
        OneWireMaster::CmdResult OWOverdriveMatchRom(OneWireMaster & master, const RomId & romId)
        {
            OneWireMaster::CmdResult result;

            // use overdrive MatchROM
            master.OWSetSpeed(OneWireMaster::StandardSpeed);

            result = master.OWReset();
            if (result == OneWireMaster::Success)
            {
                result = master.OWWriteByte(OverdriveMatchRomCmd);
                if (result == OneWireMaster::Success)
                {
                    master.OWSetSpeed(OneWireMaster::OverdriveSpeed);
                    // send ROM
                    result = master.OWWriteBlock(romId.buffer.data(), romId.buffer.size());
                }
            }
            return result;
        }
        
        OneWireMaster::CmdResult OWResume(OneWireMaster & master)
        {
            OneWireMaster::CmdResult result;

            result = master.OWReset();
            if (result == OneWireMaster::Success)
            {
                result = master.OWWriteByte(ResumeCmd);
            }

            return result;
        }

        OneWireMaster::CmdResult OWSearch(OneWireMaster & master, SearchState & searchState)
        {
            return OWSearchAll(master, searchState, false);
        }
        
        OneWireMaster::CmdResult OWAlarmSearch(OneWireMaster & master, SearchState & searchState)
        {
            return OWSearchAll(master, searchState, true);
        }
        
        OneWireMaster::CmdResult OWFirstAlarm(OneWireMaster & master, SearchState & searchState)
        {
            searchState.reset();
            return OWAlarmSearch(master, searchState);
        }
        
        OneWireMaster::CmdResult OWNextAlarm(OneWireMaster & master, SearchState & searchState)
        {
            return OWAlarmSearch(master, searchState);
        }

        OneWireMaster::CmdResult OWSearchAll(OneWireMaster & master, SearchState & searchState, bool alarmSearch)
        {
            uint8_t id_bit_number;
            uint8_t last_zero, rom_byte_number;
            uint8_t id_bit, cmp_id_bit;
            uint8_t rom_byte_mask;
            bool search_result;
            uint8_t crc8 = 0;
            OneWireMaster::SearchDirection search_direction;

            // initialize for search
            id_bit_number = 1;
            last_zero = 0;
            rom_byte_number = 0;
            rom_byte_mask = 1;
            search_result = false;

            // if the last call was not the last one
            if (!searchState.last_device_flag)
            {
                // 1-Wire reset
                OneWireMaster::CmdResult result = master.OWReset();
                if (result != OneWireMaster::Success)
                {
                    // reset the search
                    searchState.reset();
                    return result;
                }

                // issue the search command 
                if(alarmSearch){
                    master.OWWriteByte(AlarmSearchCmd);
                }
                else{
                    master.OWWriteByte(SearchRomCmd);
                }

                // loop to do the search
                do
                {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < searchState.last_discrepancy)
                    {
                        if ((searchState.romId.buffer[rom_byte_number] & rom_byte_mask) > 0)
                        {
                            search_direction = OneWireMaster::WriteOne;
                        }
                        else
                        {
                            search_direction = OneWireMaster::WriteZero;
                        }
                    }
                    else
                    {
                        // if equal to last pick 1, if not then pick 0
                        if (id_bit_number == searchState.last_discrepancy)
                        {
                            search_direction = OneWireMaster::WriteOne;
                        }
                        else
                        {
                            search_direction = OneWireMaster::WriteZero;
                        }
                    }

                    // Peform a triple operation on the DS2465 which will perform 2 read bits and 1 write bit
                    result = master.OWTriplet(search_direction, id_bit, cmp_id_bit);
                    if (result != OneWireMaster::Success)
                    {
                        return result;
                    }

                    // check for no devices on 1-wire
                    if (id_bit && cmp_id_bit)
                    {
                        break;
                    }
                    else
                    {
                        if (!id_bit && !cmp_id_bit && (search_direction == OneWireMaster::WriteZero))
                        {
                            last_zero = id_bit_number;

                            // check for Last discrepancy in family
                            if (last_zero < 9)
                            {
                                searchState.last_family_discrepancy = last_zero;
                            }
                        }

                        // set or clear the bit in the ROM byte rom_byte_number
                        // with mask rom_byte_mask
                        if (search_direction == OneWireMaster::WriteOne)
                        {
                            searchState.romId.buffer[rom_byte_number] |= rom_byte_mask;
                        }
                        else
                        {
                            searchState.romId.buffer[rom_byte_number] &= (uint8_t)~rom_byte_mask;
                        }

                        // increment the byte counter id_bit_number
                        // and shift the mask rom_byte_mask
                        id_bit_number++;
                        rom_byte_mask <<= 1;

                        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                        if (rom_byte_mask == 0)
                        {
                            crc8 = calculateCrc8(crc8, searchState.romId.buffer[rom_byte_number]);  // accumulate the CRC
                            rom_byte_number++;
                            rom_byte_mask = 1;
                        }
                    }
                } while (rom_byte_number < searchState.romId.buffer.size());  // loop until through all ROM bytes 0-7

                // if the search was successful then
                if (!((id_bit_number <= (searchState.romId.buffer.size() * 8)) || (crc8 != 0)))
                {
                    // search successful so set m_last_discrepancy,m_last_device_flag,search_result
                    searchState.last_discrepancy = last_zero;

                    // check for last device
                    if (searchState.last_discrepancy == 0)
                    {
                        searchState.last_device_flag = true;
                    }

                    search_result = true;
                }
            }

            // if no device found then reset counters so next 'search' will be like a first
            if (!search_result || (searchState.romId.familyCode() == 0))
            {
                searchState.reset();
                search_result = false;
            }

            return (search_result ? OneWireMaster::Success : OneWireMaster::OperationFailure);
        }
    }
}
