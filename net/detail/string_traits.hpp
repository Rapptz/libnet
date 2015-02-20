// The MIT License (MIT)

// Copyright (c) 2015 Danny "Rapptz" Y.

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef LIBNET_DETAIL_STRING_TRAITS_HPP
#define LIBNET_DETAIL_STRING_TRAITS_HPP

#include <string>
#include <cstddef>

namespace net {
namespace detail {
template<typename T>
struct string_traits {
    // static_assert(std::is_trivially_copyable<T>::value, "Object must meet is_trivially_copyable trait");
    // due to it being unsupported in GCC I'm not going to impose a restriction on this yet.

    static const char* c_str(const T& obj) noexcept {
        char buf[sizeof(obj)];
        std::char_traits<char>::copy(buf, reinterpret_cast<char*>(&obj), sizeof(obj));
        return buf;
    }

    static size_t size(const T& obj) noexcept {
        return sizeof(obj);
    }
};

template<typename... Rest>
struct string_traits<std::basic_string<char, Rest...>> {
    static const char* c_str(const std::basic_string<char, Rest...>& str) noexcept {
        return str.c_str();
    }

    static size_t size(const std::basic_string<char, Rest...>& str) noexcept {
        return str.size();
    }
};

template<size_t N>
struct string_traits<char[N]> {
    static const char* c_str(const char (&arr)[N]) noexcept {
        return arr;
    }

    static size_t size(const char (&arr)[N]) noexcept {
        return N - 1;
    }
};

template<>
struct string_traits<const char*> {
    static const char* c_str(const char* str) noexcept {
        return str;
    }

    static size_t size(const char* str) noexcept {
        return std::char_traits<char>::length(str);
    }
};
} // detail
} // net

#endif // LIBNET_DETAIL_STRING_TRAITS_HPP
