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

#ifndef OneWire_Authenticators_DS28E15_22_25
#define OneWire_Authenticators_DS28E15_22_25

#include "Utilities/array.h"
#include "Slaves/Authenticators/ISha256MacCoproc.h"
#include "Slaves/OneWireSlave.h"

namespace OneWire
{
    /// Interface to the DS28E15/22/25 series of authenticators including low power variants.
    class DS28E15_22_25 : public OneWireSlave
    {
    public:
        /// Holds the contents of a device memory segment.
        typedef array<uint8_t, 4> Segment;

        /// Holds the contents of a device memory page.
        typedef array<uint8_t, 32> Page;

        /// Holds the contents of the device scratchpad.
        typedef array<uint8_t, 32> Scratchpad;

        /// Container for a SHA-256 MAC.
        typedef array<uint8_t, 32> Mac;

        /// Container for a manufacturer ID.
        typedef array<uint8_t, 2> ManId;

        /// Container for the device personality.
        class Personality
        {
        public:
            typedef array<uint8_t, 4> Buffer;
            
        private:
            Buffer m_data;

        public:
            Personality() { }
            explicit Personality(const Buffer & dataBytes) : m_data(dataBytes) { }
           
            uint8_t PB1() const { return m_data[0]; }
            uint8_t PB2() const { return m_data[1]; }
            ManId manId() const { ManId manId = { m_data[2], m_data[3] }; return manId; }
            bool secretLocked() const { return (PB2() & 0x01); }
            
            bool operator==(const Personality & rhs) const { return (this->m_data == rhs.m_data); }
            bool operator!=(const Personality & rhs) const { return !operator==(rhs); }
        };

        /// Represents the status of a memory protection block.
        class BlockProtection
        {
        private:
            static const uint8_t readProtectionMask = 0x80, writeProtectionMask = 0x40, eepromEmulationMask = 0x20, authProtectionMask = 0x10, blockNumMask = 0x0F;
            uint8_t m_status;

        public:
            explicit BlockProtection(uint8_t status = 0x00) : m_status(status) { }
            BlockProtection(bool readProtection, bool writeProtection, bool eepromEmulation, bool authProtection, unsigned int blockNum);

            /// Get the byte representation used by the device.
            uint8_t statusByte() const { return m_status; }
            /// Set the byte representation used by the device.
            void setStatusByte(uint8_t status) { m_status = status; }

            /// Get the Block Number which is indexed from zero.
            unsigned int blockNum() const { return (m_status & blockNumMask); }
            /// Set the Block Number which is indexed from zero.
            void setBlockNum(unsigned int blockNum);

            /// Get the Read Protection status.
            /// @returns True if Read Protection is enabled.
            bool readProtection() const { return ((m_status & readProtectionMask) == readProtectionMask); }
            /// Set the Read Protection status.
            void setReadProtection(bool readProtection);

            /// Get the Write Protection status.
            /// @returns True if Write Protection is enabled.
            bool writeProtection() const { return ((m_status & writeProtectionMask) == writeProtectionMask); }
            /// Set the Write Protection status.
            void setWriteProtection(bool writeProtection);

            /// Get the EEPROM Emulation Mode status.
            /// @returns True if EEPROM Emulation Mode is enabled.
            bool eepromEmulation() const { return ((m_status & eepromEmulationMask) == eepromEmulationMask); }
            /// Set the EEPROM Emulation Mode status.
            void setEepromEmulation(bool eepromEmulation);

            /// Get the Authentication Protection status.
            /// @returns True if Authentication Protection is enabled.
            bool authProtection() const { return ((m_status & authProtectionMask) == authProtectionMask); }
            /// Set the Authentication Protection status.
            void setAuthProtection(bool authProtection);

            /// Check if no protection options are enabled.
            /// @returns True if no protection options are enabled.
            bool noProtection() const;

            bool operator==(const BlockProtection & rhs) const { return (this->m_status == rhs.m_status); }
            bool operator!=(const BlockProtection & rhs) const { return !operator==(rhs); }
        };

        /// Compute the MAC for an Authenticated Write to a memory segment.
        /// @param MacCoproc Coprocessor with Slave Secret to use for the computation.
        /// @param pageNum Page number for write operation.
        /// @param segmentNum Segment number within page for write operation.
        /// @param[in] newData New data to write to the segment.
        /// @param[in] oldData Existing data contained in the segment.
        /// @param[in] romId 1-Wire ROM ID of the device.
        /// @param[in] manId Manufacturer ID of the device.
        /// @param[out] mac The computed MAC.
        /// @returns The result code indicated by the coprocessor.
        static ISha256MacCoproc::CmdResult computeSegmentWriteMac(const ISha256MacCoproc & MacCoproc,
                                                                  unsigned int pageNum,
                                                                  unsigned int segmentNum,
                                                                  const Segment & newData,
                                                                  const Segment & oldData,
                                                                  const RomId & romId,
                                                                  const ManId & manId,
                                                                  Mac & mac);

        /// Compute the MAC for an Authenticated Write to a memory protection block.
        /// @param MacCoproc Coprocessor with Slave Secret to use for the operation.
        /// @param[in] newProtection New protection status to write.
        /// @param[in] oldProtection Existing protection status in device.
        /// @param[in] romId 1-Wire ROM ID of the device.
        /// @param[in] manId Manufacturer ID of the device.
        /// @param[out] mac The computed MAC.
        /// @returns The result code indicated by the coprocessor.
        static ISha256MacCoproc::CmdResult computeProtectionWriteMac(const ISha256MacCoproc & MacCoproc,
                                                                     const BlockProtection & newProtection,
                                                                     const BlockProtection & oldProtection,
                                                                     const RomId & romId,
                                                                     const ManId & manId,
                                                                     Mac & mac);

        /// Compute the next secret from the existing secret.
        /// @param MacCoproc Coprocessor with Master Secret to use for the operation.
        ///        Slave Secret will be updated with the computation result.
        /// @param[in] bindingPage Binding data from a device memory page.
        /// @param bindingPageNum Number of the page where the binding data is from.
        /// @param[in] partialSecret Partial secret data from the device scratchpad.
        /// @param[in] romId 1-Wire ROM ID of the device.
        /// @param[in] manId Manufacturer ID of the device.
        /// @returns The result code indicated by the coprocessor.
        static ISha256MacCoproc::CmdResult computeNextSecret(ISha256MacCoproc & MacCoproc,
                                                             const Page & bindingPage,
                                                             unsigned int bindingPageNum,
                                                             const Scratchpad & partialSecret,
                                                             const RomId & romId,
                                                             const ManId & manId);

        /// Compute a Page MAC for authentication.
        /// @param MacCoproc Coprocessor with Slave Secret to use for the operation.
        /// @param[in] pageData Data from a device memory page.
        /// @param pageNum Number of the page to use data from.
        /// @param[in] challenge Random challenge to prevent replay attacks.
        /// @param[in] romId 1-Wire ROM ID of the device.
        /// @param[in] manId Manufacturer ID of the device.
        /// @param[out] mac The computed MAC.
        static ISha256MacCoproc::CmdResult computeAuthMac(const ISha256MacCoproc & MacCoproc,
                                                          const Page & pageData,
                                                          unsigned int pageNum,
                                                          const Scratchpad & challenge,
                                                          const RomId & romId,
                                                          const ManId & manId,
                                                          Mac & mac);

        /// Compute a Page MAC for authentication using anonymous mode.
        /// @param MacCoproc Coprocessor with Slave Secret to use for the operation.
        /// @param[in] pageData Data from a device memory page.
        /// @param pageNum Number of the page to use data from.
        /// @param[in] challenge Random challenge to prevent replay attacks.
        /// @param[in] manId Manufacturer ID of the device.
        /// @param[out] mac The computed MAC.
        static ISha256MacCoproc::CmdResult computeAuthMacAnon(const ISha256MacCoproc & MacCoproc,
                                                              const Page & pageData,
                                                              unsigned int pageNum,
                                                              const Scratchpad & challenge,
                                                              const ManId & manId,
                                                              Mac & mac);

        /// Number of segments per page.
        static const unsigned int segmentsPerPage = (Page::csize / Segment::csize);
        
        /// Creates a segment representation from a subsection of the page data.
        /// @param segmentNum Segment number within page to copy from.
        /// @returns The copied segment data.
        static Segment segmentFromPage(unsigned int segmentNum, const Page & page);

        /// Copies segment data to the page.
        /// @param segmentNum Segment number within the page to copy to.
        /// @param[in] segment Segment to copy from.
        static void segmentToPage(unsigned int segmentNum, const Segment & segment, Page & page);

        /// @{
        /// Manufacturer ID
        ManId manId() const { return m_manId; }
        void setManId(const ManId & manId) { m_manId = manId; }
        /// @}

        /// @{
        /// Enable low voltage timing
        bool lowVoltage() const { return m_lowVoltage; }
        void setLowVoltage(bool lowVoltage) { m_lowVoltage = lowVoltage; }
        /// @}

        // Const member functions should not affect the state of the memory, block protection, or secret on the DS28Exx.
        // Scratchpad on the DS28Exx is considered mutable.

        /// Perform Load and Lock Secret command on the device.
        /// @note The secret should already be stored in the scratchpad on the device.
        /// @param lock Prevent further changes to the secret on the device after loading.
        CmdResult loadSecret(bool lock);

        /// Read memory segment using the Read Memory command on the device.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number for read operation.
        /// @param segmentNum Segment number within page for read operation.
        /// @param[out] data Buffer to read data from the segment into.
        /// @param continuing True if continuing a previous Read Memory command.
        ///                   False to begin a new command.
        CmdResult readSegment(unsigned int pageNum, unsigned int segmentNum, Segment & data, bool continuing = false) const;

        /// Write memory segment using the Write Memory command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number for write operation.
        /// @param segmentNum Segment number within page for write operation.
        /// @param[in] data Data to write to the memory segment.
        /// @param continuing True to continue writing with the next sequential segment.
        ///                   False to begin a new command.
        CmdResult writeSegment(unsigned int pageNum, unsigned int segmentNum, const Segment & data, bool continuing = false);

        /// Read memory page using the Read Memory command on the device.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number for write operation.
        /// @param[out] rdbuf Buffer to read data from the page into.
        /// @param continuing True if continuing a previous Read Memory command.
        ///                   False to begin a new command.
        CmdResult readPage(unsigned int pageNum, Page & rdbuf, bool continuing = false) const;

        /// Perform a Compute and Lock Secret command on the device.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number to use as the binding data.
        /// @param lock Prevent further changes to the secret on the device after computing.
        CmdResult computeSecret(unsigned int pageNum, bool lock);

        /// Perform a Compute Page MAC command on the device.
        /// Read back the MAC and verify the CRC16.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param pageNum Page number to use for the computation.
        /// @param anon True to compute in anonymous mode where ROM ID is not used.
        /// @param[out] mac The device computed MAC.
        CmdResult computeReadPageMac(unsigned int pageNum, bool anon, Mac & mac) const;

        /// Update the status of a memory protection block using the Write Page Protection command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param[in] Desired protection status for the block.
        ///            It is not possible to disable existing protections.
        /// @param continuing True to continue a previous Write Page Protection command.
        ///                   False to begin a new command.
        CmdResult writeBlockProtection(const BlockProtection & protection);

        /// Update the status of a memory protection block using the Authenticated Write Page Protection command.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param MacCoproc Coprocessor with Slave Secret to use for the operation.
        /// @param[in] newProtection New protection status to write.
        /// @param[in] oldProtection Existing protection status in device.
        /// @param continuing True to continue a previous Authenticated Write Page Protection command.
        ///                   False to begin a new command.
        CmdResult writeAuthBlockProtection(const ISha256MacCoproc & MacCoproc,
                                           const BlockProtection & newProtection,
                                           const BlockProtection & oldProtection);
                                      
    protected:    
        /// @param owMaster 1-Wire Master to use for communication with DS28E15/22/25.
        /// @param lowVoltage Enable low voltage timing.
        DS28E15_22_25(RandomAccessRomIterator & selector, bool lowVoltage);
        
        ~DS28E15_22_25() { }
        
        template <class T>
        CmdResult doWriteScratchpad(const Scratchpad & data) const;

        template <class T>
        CmdResult doReadScratchpad(Scratchpad & data) const;
        
        template <class T>
        CmdResult doReadBlockProtection(unsigned int blockNum, BlockProtection & protection) const;

        template <class T>
        CmdResult doReadPersonality(Personality & personality) const;
        
        template <class T>
        CmdResult doWriteAuthSegment(const ISha256MacCoproc & MacCoproc,
                                     unsigned int pageNum,
                                     unsigned int segmentNum,
                                     const Segment & newData,
                                     const Segment & oldData,
                                     bool continuing);

        template <class T>
        CmdResult doWriteAuthSegmentMac(unsigned int pageNum,
                                        unsigned int segmentNum,
                                        const Segment & newData,
                                        const Mac & mac,
                                        bool continuing);
        
        template <class T, size_t N>
        CmdResult doReadAllBlockProtection(array<BlockProtection, N> & protection) const;

    private:
        /// Read status bytes which are either personality or block protection.
        /// @note 1-Wire ROM selection should have already occurred.
        /// @param personality True to read personality or false to read block protection.
        /// @param allpages True to read all pages or false to read one page.
        /// @param pageNum Page number if reading block protection.
        /// @param rdbuf Buffer to receive data read from device.
        template <class T>
        CmdResult readStatus(bool personality, bool allpages, unsigned int blockNum, uint8_t * rdbuf) const;
    
        ManId m_manId;
        bool m_lowVoltage;
    
        static const unsigned int shaComputationDelayMs = 3;
        static const unsigned int eepromWriteDelayMs = 10;
        unsigned int secretEepromWriteDelayMs() const { return (m_lowVoltage ? 200 : 100); }
    };
}

#endif
