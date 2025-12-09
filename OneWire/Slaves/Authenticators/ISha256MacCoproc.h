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

#ifndef OneWire_Authenticators_ISha256MacCoproc
#define OneWire_Authenticators_ISha256MacCoproc

#include <stddef.h>
#include <stdint.h>
#include "Utilities/array.h"

namespace OneWire
{
    /// Interface for SHA-256 coprocessors compatible with the DS28E15/22/25 family and similar.
    class ISha256MacCoproc
    {
    public:
        enum CmdResult
        {
            Success,
            OperationFailure
        };

        /// Holds the contents of a device memory page.
        typedef array<uint8_t, 32> DevicePage;

        /// Holds the contents of a device scratchpad.
        typedef array<uint8_t, 32> DeviceScratchpad;

        /// Holds the contents of a device secret.
        typedef array<uint8_t, 32> Secret;

        /// Container for a SHA-256 MAC.
        typedef array<uint8_t, 32> Mac;

        /// Additional data fields for Compute Write MAC operation.
        typedef array<uint8_t, 20> WriteMacData;

        /// Additional data fields for the Compute Auth. MAC operation.
        typedef array<uint8_t, 12> AuthMacData;

        /// Additional data field for the Compute Slave Secret operation.
        typedef array<uint8_t, 12> SlaveSecretData;

        /// Set Master Secret in coprocessor.
        /// @param[in] masterSecret New master secret to set.
        virtual CmdResult setMasterSecret(const Secret & masterSecret) = 0;

        /// Compute Slave Secret in the coprocessor.
        /// @note Uses the previously set Master Secret in computation.
        /// @param[in] devicePage Page data stored on device.
        /// @param[in] deviceScratchpad Scratchpad data stored on device.
        /// @param[in] slaveSecretData Additional data fields as specified by device.
        virtual CmdResult computeSlaveSecret(const DevicePage & devicePage, const DeviceScratchpad & deviceScratchpad, const SlaveSecretData & slaveSecretData) = 0;

        /// Compute Write MAC
        /// @note Uses the previously computed Slave Secret in computation.
        /// @param[in] writeMacData Additional data fields as specified by device.
        /// @param[out] mac The computed MAC.
        virtual CmdResult computeWriteMac(const WriteMacData & writeMacData, Mac & mac) const = 0;

        /// Compute Authentication MAC
        /// @note Uses the previously computed Slave Secret in computation.
        /// @param[in] devicePage Page data stored on device.
        /// @param[in] challege Random challenge for device.
        /// @param[in] authMacData Additional data fields as specified by device.
        /// @param[out] mac The computed MAC.
        virtual CmdResult computeAuthMac(const DevicePage & devicePage, const DeviceScratchpad & challenge, const AuthMacData & authMacData, Mac & mac) const = 0;
    };
}

#endif
