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


#ifndef ONEWIRE_MASTERS_H
#define ONEWIRE_MASTERS_H


//#include "Masters/DS248x/DS2484/DS2484.h"
//#include "Masters/DS248x/DS2482EightChannel/DS2482EightChannel.h"
//#include "Masters/DS248x/DS2482SingleChannel/DS2482SingleChannel.h"
//#include "Masters/DS2480B/DS2480B.h"
//#include "Masters/DS2465/DS2465.h"
//
//#if defined(TARGET_MAX32600)
//    #include "Masters/TARGET_Maxim/TARGET_MAX32600/OwGpio/OwGpio.h"
//#endif

#if defined(TARGET_MAX32620) || defined(TARGET_MAX32625) || defined(TARGET_MAX32630)
    #include "Masters/TARGET_Maxim/MCU_OWM.h"
#endif

#endif /*ONEWIRE_MASTERS_H*/
