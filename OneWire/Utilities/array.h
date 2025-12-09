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

#ifndef OneWire_array
#define OneWire_array

#include <stdint.h>
#include <stddef.h>
#include <iterator>
#include <algorithm>
#include <cstring>

namespace OneWire
{
    /// Generic array class similar to std::array.
    template <typename T, size_t N>
    class array
    {
    public:
        typedef T value_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type & reference;
        typedef const value_type & const_reference;
        typedef value_type * pointer;
        typedef const value_type * const_pointer;
        typedef pointer iterator;
        typedef const_pointer const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
        // Element access
        reference operator[](size_t pos) { return _buffer[pos]; }
        const_reference operator[](size_t pos) const { return _buffer[pos]; }
        reference front() { return const_cast<reference>(static_cast<const array<T, N> &>(*this).front()); }
        const_reference front() const { return _buffer[0]; }
        reference back() { return const_cast<reference>(static_cast<const array<T, N> &>(*this).back()); }
        const_reference back() const { return _buffer[N - 1]; }
        T * data() { return const_cast<T *>(static_cast<const array<T, N> &>(*this).data()); }
        const T * data() const { return _buffer; }
        
        // Iterators
        iterator begin() { return const_cast<iterator>(static_cast<const array<T, N> &>(*this).begin()); }
        const_iterator begin() const { return cbegin(); }
        const_iterator cbegin() const { return &front(); }
        iterator end() { return const_cast<iterator>(static_cast<const array<T, N> &>(*this).end()); }
        const_iterator end() const { return cend(); }
        const_iterator cend() const { return &_buffer[N]; }
        reverse_iterator rbegin() { return reverse_iterator(&back()); }
        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator crbegin() const { return const_reverse_iterator(&back()); }
        reverse_iterator rend() { return reverse_iterator(--begin()); }
        const_reverse_iterator rend() const { return crend(); }
        const_reverse_iterator crend() const { return const_reverse_iterator(--begin()); }
        
        // Capacity
        static bool empty() { return size() == 0; }
        static size_type size() { return N; }
        static size_type max_size() { return size(); }
        static const size_type csize = N; ///< Alternative to size() when a constant expression is required.
        
        // Operations
        void fill(const T & value) { std::fill(begin(), end(), value); }
        
        bool operator==(const array<T, N> & rhs) const { return (std::memcmp(this->_buffer, rhs._buffer, N * sizeof(T)) == 0); }
        bool operator!=(const array<T, N> & rhs) const { return !operator==(rhs); }
        
        T _buffer[N];
    };
}

#endif
