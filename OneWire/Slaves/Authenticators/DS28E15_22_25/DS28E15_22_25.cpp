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

#include "DS28E15_22_25.h"
#include "DS28E15.h"
#include "DS28E22.h"
#include "DS28E25.h"
#include "Masters/OneWireMaster.h"
#include "Utilities/crc.h"
#include "Utilities/type_traits.h"
#include "mbed_wait_api.h"

using namespace OneWire;
using namespace OneWire::crc;

/// 1-Wire device commands.
enum Command
{
    WriteMemory = 0x55,
    ReadMemory = 0xF0,
    LoadAndLockSecret = 0x33,
    ComputeAndLockSecret = 0x3C,
    ReadWriteScratchpad = 0x0F,
    ComputePageMac = 0xA5,
    ReadStatus = 0xAA,
    WriteBlockProtection = 0xC3,
    AuthWriteMemory = 0x5A,
    AuthWriteBlockProtection = 0xCC,
};

DS28E15_22_25::Segment DS28E15_22_25::segmentFromPage(unsigned int segmentNum, const Page & page)
{
    if (segmentNum > (segmentsPerPage - 1))
    {
        segmentNum = (segmentsPerPage - 1);
    }
    Segment segment;
    Page::const_iterator copyBegin = page.begin() + (segmentNum * Segment::size());
    std::copy(copyBegin, copyBegin + Segment::size(), segment.begin());
    return segment;
}

void DS28E15_22_25::segmentToPage(unsigned int segmentNum, const Segment & segment, Page & page)
{
    if (segmentNum > (segmentsPerPage - 1))
    {
        segmentNum = (segmentsPerPage - 1);
    }
    std::copy(segment.begin(), segment.end(), page.begin() + (segmentNum * segment.size()));
}

DS28E15_22_25::BlockProtection::BlockProtection(bool readProtection, bool writeProtection, bool eepromEmulation, bool authProtection, unsigned int blockNum)
{
    setReadProtection(readProtection);
    setWriteProtection(writeProtection);
    setEepromEmulation(eepromEmulation);
    setAuthProtection(authProtection);
    setBlockNum(blockNum);
}

void DS28E15_22_25::BlockProtection::setBlockNum(unsigned int blockNum)
{
    m_status &= ~blockNumMask;
    m_status |= (blockNum & blockNumMask);
}

bool DS28E15_22_25::BlockProtection::noProtection() const
{
    return !readProtection() && !writeProtection() && !eepromEmulation() && !authProtection();
}

void DS28E15_22_25::BlockProtection::setReadProtection(bool readProtection)
{
    if (readProtection)
    {
        m_status |= readProtectionMask;
    }
    else
    {
        m_status &= ~readProtectionMask;
    }
}

void DS28E15_22_25::BlockProtection::setWriteProtection(bool writeProtection)
{
    if (writeProtection)
    {
        m_status |= writeProtectionMask;
    }
    else
    {
        m_status &= ~writeProtectionMask;
    }
}

void DS28E15_22_25::BlockProtection::setEepromEmulation(bool eepromEmulation)
{
    if (eepromEmulation)
    {
        m_status |= eepromEmulationMask;
    }
    else
    {
        m_status &= ~eepromEmulationMask;
    }
}

void DS28E15_22_25::BlockProtection::setAuthProtection(bool authProtection)
{
    if (authProtection)
    {
        m_status |= authProtectionMask;
    }
    else
    {
        m_status &= ~authProtectionMask;
    }
}

DS28E15_22_25::DS28E15_22_25(RandomAccessRomIterator & selector, bool lowVoltage)
    : OneWireSlave(selector), m_manId(), m_lowVoltage(lowVoltage)
{

}

OneWireSlave::CmdResult DS28E15_22_25::writeAuthBlockProtection(const ISha256MacCoproc & MacCoproc, const BlockProtection & newProtection, const BlockProtection & oldProtection)
{
    uint8_t buf[256], cs;
    int cnt = 0;
    Mac mac;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = AuthWriteBlockProtection;
    buf[cnt++] = newProtection.statusByte();

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // read first CRC byte
    master().OWReadByte(buf[cnt++]);

    // read the last CRC and enable
    master().OWReadBytePower(buf[cnt++]);

    // now wait for the MAC computation.
    wait_us(1000*shaComputationDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // check CRC16
    if (calculateCrc16(buf, cnt) != 0xB001)
    {
        return CrcError;
    }

    ISha256MacCoproc::CmdResult result;
    result = computeProtectionWriteMac(MacCoproc, newProtection, oldProtection, romId(), manId(), mac);
    if (result != ISha256MacCoproc::Success)
    {
        return OperationFailure;
    }
    cnt = 0;

    // send the MAC
    master().OWWriteBlock(mac.data(), mac.size());

    // Read CRC and CS byte
    master().OWReadBlock(&buf[cnt], 3);
    cnt += 3;

    // check CRC16
    if (calculateCrc16(buf, cnt - 1, calculateCrc16(mac.data(), mac.size())) != 0xB001)
    {
        return CrcError;
    }

    // check CS
    if (buf[cnt - 1] != 0xAA)
    {
        return OperationFailure;
    }

    // send release and strong pull-up
    // DATASHEET_CORRECTION - last bit in release is a read-zero so don't check echo of write byte
    master().OWWriteBytePower(0xAA);

    // now wait for the programming.
    wait_us(1000*eepromWriteDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

OneWireSlave::CmdResult DS28E15_22_25::writeBlockProtection(const BlockProtection & protection)
{
    uint8_t buf[256], cs;
    int cnt = 0;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = WriteBlockProtection;

    // compute parameter byte 
    buf[cnt++] = protection.statusByte();

    master().OWWriteBlock(&buf[0], cnt);

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check CRC16
    if (calculateCrc16(buf, cnt) != 0xB001)
    {
        return CrcError;
    }

    // sent release
    master().OWWriteBytePower(0xAA);

    // now wait for the programming.
    wait_us(1000*eepromWriteDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doReadBlockProtection(unsigned int blockNum, BlockProtection & protection) const
{
    uint8_t buf;
    CmdResult result;
    
    result = readStatus<T>(false, false, blockNum, &buf);
    if (result == Success)
    {
        protection.setStatusByte(buf);
    }
    return result;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doReadPersonality(Personality & personality) const
{
    Personality::Buffer buffer;
    CmdResult result = readStatus<T>(true, false, 0, buffer.data());
    if (result == Success)
    {
        personality = Personality(buffer);
    }
    return result;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::readStatus(bool personality, bool allpages, unsigned int blockNum, uint8_t * rdbuf) const
{
    const size_t crcLen = 4, ds28e22_25_pagesPerBlock = 2;

    uint8_t buf[256];
    size_t cnt = 0, offset = 0;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = ReadStatus;
    if (personality)
    {
        buf[cnt++] = 0xE0;
    }
    else if (allpages)
    {
        buf[cnt++] = 0;
    }
    else
    {
        // Convert to page number for DS28E22 and DS28E25
        buf[cnt] = blockNum;
        if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
        {
            buf[cnt] *= ds28e22_25_pagesPerBlock;
        }
        cnt++;
    }

    // send the command
    master().OWWriteBlock(&buf[0], 2);

    offset = cnt + 2;

    // Set data length
    size_t rdnum;
    if (personality)
    {
        rdnum = 4;
    }
    else if (!allpages)
    {
        rdnum = 1;
    }
    else if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
    {
        rdnum = DS28E25::memoryPages; // Need to read extra data on DS28E22 to get CRC16.
    }
    else // DS28E15
    {
        rdnum = DS28E15::protectionBlocks;
    }
    rdnum += crcLen; // Add in CRC length

    // Read the bytes 
    master().OWReadBlock(&buf[cnt], rdnum);
    cnt += rdnum;

    // check the first CRC16
    if (calculateCrc16(buf, offset) != 0xB001)
    {
        return CrcError;
    }

    if (personality || allpages)
    {
        // check the second CRC16
        if (calculateCrc16(buf + offset, cnt - offset) != 0xB001)
        {
            return CrcError;
        }
    }

    // copy the data to the read buffer
    rdnum -= crcLen;
    if (allpages && (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value))
    {
        if (is_same<T, DS28E22>::value)
        {
            rdnum -= (DS28E25::memoryPages - DS28E22::memoryPages);
        }

        for (size_t i = 0; i < (rdnum / ds28e22_25_pagesPerBlock); i++)
        {
            rdbuf[i] = (buf[offset + (i * ds28e22_25_pagesPerBlock)] & 0xF0); // Upper nibble
            rdbuf[i] |= ((buf[offset + (i * ds28e22_25_pagesPerBlock)] & 0x0F) / ds28e22_25_pagesPerBlock); // Lower nibble
        }
    }
    else
    {
        std::memcpy(rdbuf, &buf[offset], rdnum);
    }

    return Success;
}

ISha256MacCoproc::CmdResult DS28E15_22_25::computeAuthMac(const ISha256MacCoproc & MacCoproc, const Page & pageData, unsigned int pageNum, const Scratchpad & challenge, const RomId & romId, const ManId & manId, Mac & mac)
{
    ISha256MacCoproc::AuthMacData authMacData;

    // insert ROM number or FF
    std::memcpy(authMacData.data(), romId.buffer.data(), romId.buffer.size());

    authMacData[10] = pageNum;

    authMacData[9] = manId[0];
    authMacData[8] = manId[1];

    authMacData[11] = 0x00;

    return MacCoproc.computeAuthMac(ISha256MacCoproc::DevicePage(pageData), challenge, authMacData, mac);
}

ISha256MacCoproc::CmdResult DS28E15_22_25::computeAuthMacAnon(const ISha256MacCoproc & MacCoproc, const Page & pageData, unsigned int pageNum, const Scratchpad & challenge, const ManId & manId, Mac & mac)
{
    RomId romId;
    romId.buffer.fill(0xFF);
    return computeAuthMac(MacCoproc, pageData, pageNum, challenge, romId, manId, mac);
}

OneWireSlave::CmdResult DS28E15_22_25::computeReadPageMac(unsigned int page_num, bool anon, Mac & mac) const
{
    uint8_t buf[256], cs;
    int cnt = 0;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = ComputePageMac;
    buf[cnt++] = ((anon) ? 0xE0 : 0x00) | page_num;

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // read first CRC byte
    master().OWReadByte(buf[cnt++]);

    // read the last CRC and enable
    master().OWReadBytePower(buf[cnt++]);

    // now wait for the MAC computation.
    wait_us(1000*shaComputationDelayMs * 2);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // check CRC16
    if (calculateCrc16(buf, cnt) != 0xB001)
    {
        return CrcError;
    }

    // read the CS byte
    master().OWReadByte(cs);
    if (cs != 0xAA)
    {
        return OperationFailure;
    }

    // read the MAC and CRC
    master().OWReadBlock(&buf[0], (Mac::size() + 2));

    // check CRC16
    if (calculateCrc16(buf, Mac::size() + 2) != 0xB001)
    {
        return CrcError;
    }

    // copy MAC to return buffer
    std::memcpy(mac.data(), buf, mac.size());

    return Success;
}

OneWireSlave::CmdResult DS28E15_22_25::computeSecret(unsigned int page_num, bool lock)
{
    uint8_t buf[256], cs;
    int cnt = 0;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = ComputeAndLockSecret;
    buf[cnt++] = (lock) ? (0xE0 | page_num) : page_num;  // lock flag 

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check CRC16
    if (calculateCrc16(buf, cnt) != 0xB001)
    {
        return CrcError;
    }

    // send release and strong pull-up
    master().OWWriteBytePower(0xAA);

    // now wait for the MAC computations and secret programming.
    wait_us(1000*shaComputationDelayMs * 2 + secretEepromWriteDelayMs());

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doWriteScratchpad(const Scratchpad & data) const
{
    uint8_t buf[256];
    int cnt = 0, offset;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = ReadWriteScratchpad;
    if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
    {
        buf[cnt++] = 0x20;
    }
    else
    {
        buf[cnt++] = 0x00;
    }

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    offset = cnt;

    // add the data
    std::memcpy(&buf[cnt], data.data(), data.size());
    cnt += data.size();

    // Send the data
    master().OWWriteBlock(data.data(), data.size());

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check first CRC16
    if (calculateCrc16(buf, offset) != 0xB001)
    {
        return CrcError;
    }

    // check the second CRC16
    if (calculateCrc16(buf + offset, cnt - offset) != 0xB001)
    {
        return CrcError;
    }

    return Success;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doReadScratchpad(Scratchpad & data) const
{
    uint8_t buf[256];
    int cnt = 0, offset;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = ReadWriteScratchpad;
    if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
    {
        buf[cnt++] = 0x2F;
    }
    else
    {
        buf[cnt++] = 0x0F;
    }

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    offset = cnt;

    // Receive the data
    master().OWReadBlock(&buf[cnt], data.size());
    cnt += data.size();

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check first CRC16
    if (calculateCrc16(buf, offset) != 0xB001)
    {
        return CrcError;
    }

    // check the second CRC16
    if (calculateCrc16(buf + offset, cnt - offset) != 0xB001)
    {
        return CrcError;
    }

    // Copy to output
    std::memcpy(data.data(), &buf[offset], data.size());

    return Success;
}

OneWireSlave::CmdResult DS28E15_22_25::loadSecret(bool lock)
{
    uint8_t buf[256], cs;
    int cnt = 0;
    
    if (selectDevice() != OneWireMaster::Success)
    {
        return CommunicationError;
    }

    buf[cnt++] = LoadAndLockSecret;
    buf[cnt++] = (lock) ? 0xE0 : 0x00;  // lock flag 

    // Send command
    master().OWWriteBlock(&buf[0], 2);

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check CRC16
    if (calculateCrc16(buf, cnt) != 0xB001)
    {
        return CrcError;
    }

    // send release and strong pull-up
    master().OWWriteBytePower(0xAA);

    // now wait for the secret programming.
    wait_us(1000*secretEepromWriteDelayMs());

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

OneWireSlave::CmdResult DS28E15_22_25::readPage(unsigned int page, Page & rdbuf, bool continuing) const
{
    uint8_t buf[256];
    int cnt = 0;
    int offset = 0;

    // check if not continuing a previous block write
    if (!continuing)
    {
        if (selectDevice() != OneWireMaster::Success)
        {
            return CommunicationError;
        }
        
        buf[cnt++] = ReadMemory;
        buf[cnt++] = page;   // address 

        // Send command
        master().OWWriteBlock(&buf[0], 2);

        // Read CRC
        master().OWReadBlock(&buf[cnt], 2);
        cnt += 2;

        offset = cnt;
    }

    // read data and CRC16
    master().OWReadBlock(&buf[cnt], (rdbuf.size() + 2));
    cnt += 34;

    // check the first CRC16
    if (!continuing)
    {
        if (calculateCrc16(buf, offset) != 0xB001)
        {
            return CrcError;
        }
    }

    // check the second CRC16
    if (calculateCrc16(buf + offset, cnt - offset) != 0xB001)
    {
        return CrcError;
    }

    // copy the data to the read buffer
    std::memcpy(rdbuf.data(), &buf[offset], rdbuf.size());

    return Success;
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doWriteAuthSegmentMac(unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Mac & mac, bool continuing)
{
    uint8_t buf[256], cs;
    int cnt, offset;

    cnt = 0;
    offset = 0;

    // check if not continuing a previous block write
    if (!continuing)
    {
        if (selectDevice() != OneWireMaster::Success)
        {
            return CommunicationError;
        }
        
        buf[cnt++] = AuthWriteMemory;
        buf[cnt++] = (segmentNum << 5) | pageNum;   // address 

        // Send command
        master().OWWriteBlock(&buf[0], 2);

        // Read CRC
        master().OWReadBlock(&buf[cnt], 2);
        cnt += 2;

        offset = cnt;
    }

    // add the data
    for (size_t i = 0; i < newData.size(); i++)
    {
        buf[cnt++] = newData[i];
    }

    // Send data
    master().OWWriteBlock(newData.data(), newData.size());

    // read first CRC byte
    master().OWReadByte(buf[cnt++]);

    // read the last CRC and enable power
    master().OWReadBytePower(buf[cnt++]);

    // now wait for the MAC computation.
    wait_us(1000*shaComputationDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // check the first CRC16
    if (!continuing)
    {
        if (calculateCrc16(buf, offset) != 0xB001)
        {
            return CrcError;
        }
    }

    // check the second CRC16
    uint16_t CRC16 = 0;

    // DS28E25/DS28E22, crc gets calculagted with CS byte 
    if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
    {
        if (continuing)
        {
            CRC16 = calculateCrc16(CRC16, 0xAA);
        }
    }

    CRC16 = calculateCrc16(buf + offset, cnt - offset, CRC16);

    if (CRC16 != 0xB001)
    {
        return CrcError;
    }

    // transmit MAC as a block
    master().OWWriteBlock(mac.data(), mac.size());

    // calculate CRC on MAC
    CRC16 = calculateCrc16(mac.data(), mac.size());

    // append read of CRC16 and CS byte
    master().OWReadBlock(&buf[0], 3);
    cnt = 3;

    // ckeck CRC16
    CRC16 = calculateCrc16(buf, cnt - 1, CRC16);

    if (CRC16 != 0xB001)
    {
        return CrcError;
    }

    // check CS
    if (buf[cnt - 1] != 0xAA)
    {
        return OperationFailure;
    }

    // send release and strong pull-up
    master().OWWriteBytePower(0xAA);

    // now wait for the programming.
    wait_us(1000*eepromWriteDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

ISha256MacCoproc::CmdResult DS28E15_22_25::computeSegmentWriteMac(const ISha256MacCoproc & MacCoproc, unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Segment & oldData, const RomId & romId, const ManId & manId, Mac & mac)
{
    ISha256MacCoproc::WriteMacData MT;

    // insert ROM number
    std::memcpy(&MT[0], romId.buffer.data(), romId.buffer.size());

    MT[11] = segmentNum;
    MT[10] = pageNum;
    MT[9] = manId[0];
    MT[8] = manId[1];

    // insert old data
    std::memcpy(&MT[12], oldData.data(), oldData.size());

    // insert new data
    std::memcpy(&MT[16], newData.data(), newData.size());

    return MacCoproc.computeWriteMac(MT, mac);
}

ISha256MacCoproc::CmdResult DS28E15_22_25::computeProtectionWriteMac(const ISha256MacCoproc & MacCoproc, const BlockProtection & newProtection, const BlockProtection & oldProtection, const RomId & romId, const ManId & manId, Mac & mac)
{
    ISha256MacCoproc::WriteMacData MT;

    // insert ROM number
    std::memcpy(MT.data(), romId.buffer.data(), romId.buffer.size());

    // instert block and page
    MT[11] = 0;
    MT[10] = newProtection.blockNum();

    MT[9] = manId[0];
    MT[8] = manId[1];

    // old data
    MT[12] = oldProtection.authProtection() ? 0x01 : 0x00;
    MT[13] = oldProtection.eepromEmulation() ? 0x01 : 0x00;
    MT[14] = oldProtection.writeProtection() ? 0x01 : 0x00;
    MT[15] = oldProtection.readProtection() ? 0x01 : 0x00;
    // new data
    MT[16] = newProtection.authProtection() ? 0x01 : 0x00;
    MT[17] = newProtection.eepromEmulation() ? 0x01 : 0x00;
    MT[18] = newProtection.writeProtection() ? 0x01 : 0x00;
    MT[19] = newProtection.readProtection() ? 0x01 : 0x00;

    // compute the mac
    return MacCoproc.computeWriteMac(MT, mac);
}

template <class T>
OneWireSlave::CmdResult DS28E15_22_25::doWriteAuthSegment(const ISha256MacCoproc & MacCoproc, unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Segment & oldData, bool continuing)
{
    uint8_t buf[256], cs;
    int cnt, offset;

    cnt = 0;
    offset = 0;

    // check if not continuing a previous block write
    if (!continuing)
    {
        if (selectDevice() != OneWireMaster::Success)
        {
            return CommunicationError;
        }
        
        buf[cnt++] = AuthWriteMemory;
        buf[cnt++] = (segmentNum << 5) | pageNum;   // address 

        // Send command
        master().OWWriteBlock(&buf[0], 2);

        // Read CRC
        master().OWReadBlock(&buf[cnt], 2);
        cnt += 2;

        offset = cnt;
    }

    // add the data
    for (size_t i = 0; i < newData.size(); i++)
    {
        buf[cnt++] = newData[i];
    }

    // Send data
    master().OWWriteBlock(newData.data(), newData.size());

    // read first CRC byte
    master().OWReadByte(buf[cnt++]);

    // read the last CRC and enable power
    master().OWReadBytePower(buf[cnt++]);

    // now wait for the MAC computation.
    wait_us(1000*shaComputationDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // check the first CRC16
    if (!continuing)
    {
        if (calculateCrc16(buf, offset) != 0xB001)
        {
            return CrcError;
        }
    }

    // check the second CRC16
    uint16_t CRC16 = 0;

    // DS28E25/DS28E22, crc gets calculated with CS byte
    if (is_same<T, DS28E22>::value || is_same<T, DS28E25>::value)
    {
        if (continuing)
            CRC16 = calculateCrc16(CRC16, 0xAA);
    }

    CRC16 = calculateCrc16(buf + offset, cnt - offset, CRC16);

    if (CRC16 != 0xB001)
    {
        return CrcError;
    }

    // compute the mac
    ISha256MacCoproc::CmdResult result;
    Mac mac;
    result = computeSegmentWriteMac(MacCoproc, pageNum, segmentNum, newData, oldData, romId(), manId(), mac);
    if (result != ISha256MacCoproc::Success)
    {
        return OperationFailure;
    }

    // transmit MAC as a block
    master().OWWriteBlock(mac.data(), mac.size());

    // calculate CRC on MAC
    CRC16 = calculateCrc16(mac.data(), mac.size());

    // append read of CRC16 and CS byte
    master().OWReadBlock(&buf[0], 3);
    cnt = 3;

    // ckeck CRC16
    CRC16 = calculateCrc16(buf, cnt - 1, CRC16);

    if (CRC16 != 0xB001)
    {
        return CrcError;
    }

    // check CS
    if (buf[cnt - 1] != 0xAA)
    {
        return OperationFailure;
    }

    // send release and strong pull-up
    master().OWWriteBytePower(0xAA);

    // now wait for the programming.
    wait_us(1000*eepromWriteDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

OneWireSlave::CmdResult DS28E15_22_25::readSegment(unsigned int page, unsigned int segment, Segment & data, bool continuing) const
{
    OneWireMaster::CmdResult result = OneWireMaster::OperationFailure;
    uint8_t buf[2];

    if (!continuing)
    {
        if (selectDevice() != OneWireMaster::Success)
        {
            return CommunicationError;
        }
        
        buf[0] = ReadMemory;
        buf[1] = (segment << 5) | page;

        // Transmit command
        master().OWWriteBlock(buf, 2);

        // Receive CRC
        result = master().OWReadBlock(buf, 2);
    }
    else if (segment == 0)
    {
        // Receive CRC from previous read
        result = master().OWReadBlock(buf, 2);
    }

    // Receive data
    if (result == OneWireMaster::Success)
    {
        result = master().OWReadBlock(data.data(), data.size());
    }

    return (result == OneWireMaster::Success ? OneWireSlave::Success : OneWireSlave::CommunicationError);
}

OneWireSlave::CmdResult DS28E15_22_25::writeSegment(unsigned int page, unsigned int block, const Segment & data, bool continuing)
{
    uint8_t buf[256], cs;
    int cnt = 0;
    int offset = 0;

    cnt = 0;
    offset = 0;

    // check if not continuing a previous block write
    if (!continuing)
    {
        if (selectDevice() != OneWireMaster::Success)
        {
            return CommunicationError;
        }
        
        buf[cnt++] = WriteMemory;
        buf[cnt++] = (block << 5) | page;   // address 

        // Send command 
        master().OWWriteBlock(&buf[0], 2);

        // Read CRC
        master().OWReadBlock(&buf[cnt], 2);
        cnt += 2;

        offset = cnt;
    }

    // add the data
    for (size_t i = 0; i < data.size(); i++)
    {
        buf[cnt++] = data[i];
    }

    // Send data
    master().OWWriteBlock(data.data(), data.size());

    // Read CRC
    master().OWReadBlock(&buf[cnt], 2);
    cnt += 2;

    // check the first CRC16
    if (!continuing)
    {
        if (calculateCrc16(buf, offset) != 0xB001)
        {
            return CrcError;
        }
    }

    // check the second CRC16
    if (calculateCrc16(buf + offset, cnt - offset) != 0xB001)
    {
        return CrcError;
    }

    // send release and strong pull-up
    master().OWWriteBytePower(0xAA);

    // now wait for the programming.
    wait_us(1000*eepromWriteDelayMs);

    // disable strong pullup
    master().OWSetLevel(OneWireMaster::NormalLevel);

    // read the CS byte
    master().OWReadByte(cs);

    if (cs == 0xAA)
    {
        return Success;
    }
    // else
    return OperationFailure;
}

ISha256MacCoproc::CmdResult DS28E15_22_25::computeNextSecret(ISha256MacCoproc & MacCoproc, const Page & bindingPage, unsigned int bindingPageNum, const Scratchpad & partialSecret, const RomId & romId, const ManId & manId)
{
    ISha256MacCoproc::SlaveSecretData slaveSecretData;

    // insert ROM number
    std::memcpy(slaveSecretData.data(), romId.buffer.data(), romId.buffer.size());

    slaveSecretData[11] = 0x00;
    slaveSecretData[10] = bindingPageNum;
    slaveSecretData[9] = manId[0];
    slaveSecretData[8] = manId[1];

    return MacCoproc.computeSlaveSecret(ISha256MacCoproc::DevicePage(bindingPage), partialSecret, slaveSecretData);
}

template <class T, size_t N>
OneWireSlave::CmdResult DS28E15_22_25::doReadAllBlockProtection(array<BlockProtection, N> & protection) const
{
    uint8_t buf[N];
    CmdResult result = readStatus<T>(false, true, 0, buf);
    if (result == Success)
    {
        for (size_t i = 0; i < N; i++)
        {
            protection[i].setStatusByte(buf[i]);
        }
    }
    return result;
}

OneWireSlave::CmdResult DS28E15::writeScratchpad(const Scratchpad & data) const
{
    return doWriteScratchpad<DS28E15>(data);
}

OneWireSlave::CmdResult DS28E15::readScratchpad(Scratchpad & data) const
{
    return doReadScratchpad<DS28E15>(data);
}

OneWireSlave::CmdResult DS28E15::readBlockProtection(unsigned int blockNum, BlockProtection & protection) const
{
    return doReadBlockProtection<DS28E15>(blockNum, protection);
}

OneWireSlave::CmdResult DS28E15::readPersonality(Personality & personality) const
{
    return doReadPersonality<DS28E15>(personality);
}

OneWireSlave::CmdResult DS28E15::writeAuthSegment(const ISha256MacCoproc & MacCoproc, unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Segment & oldData, bool continuing)
{
    return doWriteAuthSegment<DS28E15>(MacCoproc, pageNum, segmentNum, newData, oldData, continuing);
}

OneWireSlave::CmdResult DS28E15::writeAuthSegmentMac(unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Mac & mac, bool continuing)
{
    return doWriteAuthSegmentMac<DS28E15>(pageNum, segmentNum, newData, mac, continuing);
}

OneWireSlave::CmdResult DS28E15::readAllBlockProtection(array<BlockProtection, protectionBlocks> & protection) const
{
    return doReadAllBlockProtection<DS28E15>(protection);
}

OneWireSlave::CmdResult DS28E22::writeScratchpad(const Scratchpad & data) const
{
    return doWriteScratchpad<DS28E22>(data);
}

OneWireSlave::CmdResult DS28E22::readScratchpad(Scratchpad & data) const
{
    return doReadScratchpad<DS28E22>(data);
}

OneWireSlave::CmdResult DS28E22::readBlockProtection(unsigned int blockNum, BlockProtection & protection) const
{
    return doReadBlockProtection<DS28E22>(blockNum, protection);
}

OneWireSlave::CmdResult DS28E22::readPersonality(Personality & personality) const
{
    return doReadPersonality<DS28E22>(personality);
}

OneWireSlave::CmdResult DS28E22::writeAuthSegment(const ISha256MacCoproc & MacCoproc, unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Segment & oldData, bool continuing)
{
    return doWriteAuthSegment<DS28E22>(MacCoproc, pageNum, segmentNum, newData, oldData, continuing);
}

OneWireSlave::CmdResult DS28E22::writeAuthSegmentMac(unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Mac & mac, bool continuing)
{
    return doWriteAuthSegmentMac<DS28E22>(pageNum, segmentNum, newData, mac, continuing);
}

OneWireSlave::CmdResult DS28E22::readAllBlockProtection(array<BlockProtection, protectionBlocks> & protection) const
{
    return doReadAllBlockProtection<DS28E22>(protection);
}

OneWireSlave::CmdResult DS28E25::writeScratchpad(const Scratchpad & data) const
{
    return doWriteScratchpad<DS28E25>(data);
}

OneWireSlave::CmdResult DS28E25::readScratchpad(Scratchpad & data) const
{
    return doReadScratchpad<DS28E25>(data);
}

OneWireSlave::CmdResult DS28E25::readBlockProtection(unsigned int blockNum, BlockProtection & protection) const
{
    return doReadBlockProtection<DS28E25>(blockNum, protection);
}

OneWireSlave::CmdResult DS28E25::readPersonality(Personality & personality) const
{
    return doReadPersonality<DS28E25>(personality);
}

OneWireSlave::CmdResult DS28E25::writeAuthSegment(const ISha256MacCoproc & MacCoproc, unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Segment & oldData, bool continuing)
{
    return doWriteAuthSegment<DS28E25>(MacCoproc, pageNum, segmentNum, newData, oldData, continuing);
}

OneWireSlave::CmdResult DS28E25::writeAuthSegmentMac(unsigned int pageNum, unsigned int segmentNum, const Segment & newData, const Mac & mac, bool continuing)
{
    return doWriteAuthSegmentMac<DS28E25>(pageNum, segmentNum, newData, mac, continuing);
}

OneWireSlave::CmdResult DS28E25::readAllBlockProtection(array<BlockProtection, protectionBlocks> & protection) const
{
    return doReadAllBlockProtection<DS28E25>(protection);
}