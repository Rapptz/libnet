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

#ifndef LIBNET_DETAIL_SOCKET_TRAITS_HPP
#define LIBNET_DETAIL_SOCKET_TRAITS_HPP

// This header is used to bridge the gap between platform differences (i.e. POSIX vs Windows)

#include <net/detail/init.hpp>
#include <net/error.hpp>
#if !defined(LIBNET_WINDOWS)
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#endif // !LIBNET_WINDOWS

namespace net {
namespace detail {
struct socket_traits {
#if defined(LIBNET_WINDOWS)
    using value_type = SOCKET;
    using getopt_type = char*;
    static constexpr value_type invalid = INVALID_SOCKET;

    static inline int close(value_type fd) {
        return ::closesocket(fd);
    }
#else
    using value_type = int;
    static constexpr value_type invalid = -1;
    using getopt_type = void*;

    static inline int close(value_type fd) {
        return ::close(fd);
    }
#endif // LIBNET_WINDOWS
};
} // detail
} // net

#endif // LIBNET_DETAIL_SOCKET_TRAITS_HPP
