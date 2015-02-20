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

#ifndef LIBNET_DETAIL_INIT_HPP
#define LIBNET_DETAIL_INIT_HPP

#include <net/detail/config.hpp>

#if LIBNET_WINDOWS
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <system_error>

namespace net {
namespace detail {
struct init {
private:
    static std::atomic<int> counter;
public:
    init() {
        if(++counter == 1) {
            WSADATA wsa;
            std::error_code ec{WSAStartup(MAKEWORD(2, 2), &wsa), std::system_category()};
            if(ec) {
                throw std::system_error(ec, "WinSock has failed to initialise");
            }
        }
    }

    ~init() {
        if(--counter == 0) {
            WSACleanup();
        }
    }
};

std::atomic<int> init::counter{0};
static init winsock_init;
} // detail
} // net

#endif  // LIBNET_WINDOWS
#endif // LIBNET_DETAIL_INIT_HPP
