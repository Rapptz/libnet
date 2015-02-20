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

#ifndef LIBNET_UTILITY_HPP
#define LIBNET_UTILITY_HPP

#include <net/detail/init.hpp>
#include <cstdint>
#if !defined(LIBNET_WINDOWS)
#include <netinet/in.h>
#endif // LIBNET_WINDOWS

namespace net {
template<typename T>
inline T host_to_network(T) noexcept = delete;

template<>
inline uint32_t host_to_network(uint32_t host) noexcept {
    return htonl(host);
}

template<>
inline uint16_t host_to_network(uint16_t host) noexcept {
    return htons(host);
}

template<typename T>
inline T network_to_host(T) noexcept = delete;

template<>
inline uint32_t network_to_host(uint32_t host) noexcept {
    return ntohl(host);
}

template<>
inline uint16_t network_to_host(uint16_t host) noexcept {
    return ntohs(host);
}
} // net

#endif // LIBNET_UTILITY_HPP
