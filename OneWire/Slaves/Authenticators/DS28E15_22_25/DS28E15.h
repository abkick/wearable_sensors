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

#ifndef OneWire_Authenticators_DS28E15
#define OneWire_Authenticators_DS28E15

#include "DS28E15_22_25.h"

namespace OneWire
{    
    /// Interface to the DS28E15 and DS28EL15 (low power) authenticators.
    class DS28E15 : public DS28E15_22_25
    {
    public:            
        // DS28E15_22_25 traits
        static const unsigned int memoryPages = 2;
        static const unsigned int protectionBlocks = 4;
    
        /// @param owMaster 1-Wire Master to use for communication with DS28E15.
        /// @param lowVoltage Enable low voltage timing.
        DS28E15(RandomAccessRomIterator & selector, bool lowVoltage = false)
            : DS28E15_22_25(selector, lowVoltage) { }
            
        /// Perform Write Scratchpad operation on the device.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param[in] data Data to write to the scratchpad.
        CmdResult writeScratchpad(const Scratchpad & data) const;

        /// Perform a Read Scratchpad operation on the device.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param[out] data Buffer to read data from the scratchpad into.
        CmdResult readScratchpad(Scratchpad & data) const;
        
        /// Read the status of a memory protection block using the Read Status command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param blockNum Block number to to read status of.
        /// @param[out] protection Receives protection status read from device.
        CmdResult readBlockProtection(unsigned int blockNum, BlockProtection & protection) const;

        /// Read the personality bytes using the Read Status command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param[out] personality Receives personality read from device.
        CmdResult readPersonality(Personality & personality) const;
        
        /// Write memory segment with authentication using the Authenticated Write Memory command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param MacCoproc Coprocessor to use for Write MAC computation.
        /// @param pageNum Page number for write operation.
        /// @param segmentNum Segment number within page for write operation.
        /// @param[in] newData New data to write to the segment.
        /// @param[in] oldData Existing data contained in the segment.
        /// @param continuing True to continue writing with the next sequential segment.
        ///                   False to begin a new command.
        CmdResult writeAuthSegment(const ISha256MacCoproc & MacCoproc,
                                   unsigned int pageNum,
                                   unsigned int segmentNum,
                                   const Segment & newData,
                                   const Segment & oldData,
                                   bool continuing = false);

        /// Write memory segment with authentication using the Authenticated Write Memory command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number for write operation.
        /// @param segmentNum Segment number within page for write operation.
        /// @param[in] newData New data to write to the segment.
        /// @param[in] mac Write MAC computed for this operation.
        /// @param continuing True to continue writing with the next sequential segment.
        ///                   False to begin a new command.
        CmdResult writeAuthSegmentMac(unsigned int pageNum,
                                      unsigned int segmentNum,
                                      const Segment & newData,
                                      const Mac & mac,
                                      bool continuing = false);
        
        /// Read the status of all memory protection blocks using the Read Status command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param[out] protection Receives protection statuses read from device.    
        CmdResult readAllBlockProtection(array<BlockProtection, protectionBlocks> & protection) const;
    };
}

#endif
