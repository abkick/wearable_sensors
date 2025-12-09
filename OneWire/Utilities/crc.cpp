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

#include "Utilities/crc.h"

namespace OneWire
{
    namespace crc
    {
        uint8_t calculateCrc8(uint8_t crc8, uint8_t data)
        {
            // See Application Note 27
            crc8 = crc8 ^ data;
            for (int i = 0; i < 8; i++)
            {
                if (crc8 & 1)
                {
                    crc8 = (crc8 >> 1) ^ 0x8c;
                }
                else
                {
                    crc8 = (crc8 >> 1);
                }
            }
         
            return crc8;
        }
         
        uint8_t calculateCrc8(const uint8_t * data, size_t dataLen, uint8_t crc)
        {
            for (size_t i = 0; i < dataLen; i++)
            {
                crc = calculateCrc8(crc, data[i]);
            }
            return crc;
        }
         
        uint16_t calculateCrc16(uint16_t crc16, uint16_t data)
        {
            const uint16_t oddparity[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

            data = (data ^ (crc16 & 0xff)) & 0xff;
            crc16 >>= 8;

            if (oddparity[data & 0xf] ^ oddparity[data >> 4])
            {
                crc16 ^= 0xc001;
            }

            data <<= 6;
            crc16 ^= data;
            data <<= 1;
            crc16 ^= data;

            return crc16;
        }

        uint16_t calculateCrc16(const uint8_t * data, size_t dataLen, uint16_t crc)
        {
            for (size_t i = 0; i < dataLen; i++)
            {
                crc = calculateCrc16(crc, data[i]);
            }
            return crc;
        }
    }
}
