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

#ifndef LIBNET_SOCKET_HPP
#define LIBNET_SOCKET_HPP

#include <net/detail/socket_traits.hpp>
#include <net/detail/string_traits.hpp>
#include <memory>

namespace net {
namespace detail {
struct addrinfo_deleter {
    void operator()(addrinfo* ptr) const noexcept {
        if(ptr != nullptr) {
            ::freeaddrinfo(ptr);
        }
    }
};
} // detail

namespace message {
enum : int {
    out_of_band = MSG_OOB,
    dont_route  = MSG_DONTROUTE,
    peek        = MSG_PEEK,
    wait_all    = MSG_WAITALL
};
} // message
struct socket {
public:
    using native_type = detail::socket_traits::value_type;
    static constexpr native_type invalid = detail::socket_traits::invalid;

    enum : int {
        stream   = SOCK_STREAM,
        datagram = SOCK_DGRAM,
        raw      = SOCK_RAW
    };

    enum : int {
        unspecified = PF_UNSPEC,
        ipv4 = PF_INET,
        ipv6 = PF_INET6
    };

    socket(int protocol, int type = socket::stream, int ip_proto = 0): fd(::socket(protocol, type, ip_proto)), protocol(protocol) {
        if(fd == invalid) {
            std::error_code ec{error::get_last_error(), error::socket_category()};
            throw std::system_error(ec);
        }
    }

    socket(const socket&) = delete;
    socket& operator=(const socket&) = delete;

    socket(socket&& other): fd(other.fd) {
        other.fd = invalid;
    }

    socket& operator=(socket&& other) {
        fd = other.fd;
        other.fd = invalid;
        return *this;
    }

    ~socket() {
        if(fd != invalid) {
            close();
        }
    }

    void close(std::error_code& ec) noexcept {
        int ret = detail::socket_traits::close(fd);

        if(ret != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
        }
        else {
            fd = invalid;
        }
    }

    void close() {
        std::error_code ec;
        close(ec);
        error::throw_on(ec, "socket::close");
    }

    int type(std::error_code& ec) const noexcept {
        return get_option<int>(SOL_SOCKET, SO_TYPE, ec);
    }

    int type() const {
        std::error_code ec;
        int ret = type(ec);
        error::throw_on(ec, "socket::type");
        return ret;
    }

    template<typename String>
    void connect(const String& host, unsigned port, std::error_code& ec) const noexcept {
        ec.clear();
        auto hints = addrinfo();
        std::unique_ptr<addrinfo, detail::addrinfo_deleter> res;
        hints.ai_family = protocol;
        hints.ai_socktype = type(ec);

        if(ec) {
            // at this point -- something went wrong so abort early.
            return;
        }

        auto port_str = std::to_string(port);
        auto ptr = res.get();
        int ret = ::getaddrinfo(detail::string_traits<String>::c_str(host), port_str.c_str(), &hints, &ptr);

        if(ret != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
            return; // abort
        }

        // loop through every result in getaddrinfo and attempt to connect
        for(addrinfo* p = ptr; p != nullptr; p = p->ai_next) {
            int ret = ::connect(fd, p->ai_addr, p->ai_addrlen);

            if(ret != 0) {
                // check for errors
                ec.assign(error::get_last_error(), error::socket_category());
            }

            // check if it's a recoverable error
            if(ec == error::host_unreachable || ec == error::network_unreachable) {
                continue;
            }
            else {
                break; // it isn't, so just finish trying.
            }
        }
    }

    template<typename String>
    void connect(const String& host, unsigned port) const {
        std::error_code ec;
        connect(host, port, ec);
        error::throw_on(ec, "socket::connect");
    }

    template<typename String>
    void bind(const String& host, unsigned port, std::error_code& ec) const noexcept {
        binder(detail::string_traits<String>::c_str(host), port, ec);
    }

    void bind(unsigned port, std::error_code& ec) const noexcept {
        binder(nullptr, port, ec);
    }

    template<typename String>
    void bind(const String& host, unsigned port) const {
        std::error_code ec;
        bind(host, port, ec);
        error::throw_on(ec, "socket::bind");
    }

    void bind(unsigned port) const {
        std::error_code ec;
        bind(port, ec);
        error::throw_on(ec, "socket::bind");
    }

    void listen(int backlog, std::error_code& ec) const noexcept {
        ec.clear();
        int ret = ::listen(fd, backlog);
        if(ret != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
        }
    }

    void listen(int backlog = 10) const {
        std::error_code ec;
        listen(backlog, ec);
        error::throw_on(ec, "socket::listen");
    }

    template<typename String>
    int send(const String& str, int flags, std::error_code& ec) const noexcept {
        ec.clear();
        using trait_type = detail::string_traits<String>;
        int ret = ::send(fd, trait_type::c_str(str), trait_type::size(str), flags);
        if(ret < 0) {
            ec.assign(error::get_last_error(), error::socket_category());
            return 0;
        }
        return ret;
    }

    template<typename String>
    int send(const String& str, int flags = 0) const {
        std::error_code ec;
        int result = send(str, flags, ec);
        error::throw_on(ec, "socket::send");
        return result;
    }

    std::string receive(int buffer_size, int flags, std::error_code& ec) const noexcept {
        if(buffer_size == 0) {
            return {}; // requested nothing so just exit.
        }

        std::string result;
        if(!error::safely_invoke([&result, &buffer_size] { result.resize(buffer_size); }, ec)) {
            return {};
        }

        int actual_bytes = ::recv(fd, &result[0], buffer_size, flags);
        if(actual_bytes < 0) {
            ec.assign(error::get_last_error(), error::socket_category());
            return {};
        }

        if(actual_bytes < buffer_size) {
            result.resize(actual_bytes);
        }
        return result;
    }

    std::string receive(int buffer_size, int flags = 0) const {
        std::error_code ec;
        auto&& result = receive(buffer_size, flags, ec);
        error::throw_on(ec, "socket::receive");
        return result;
    }
private:
    template<typename T>
    T get_option(int level, int flags, std::error_code& ec) const noexcept {
        ec.clear();
        T temp;
        auto len = static_cast<int>(sizeof(T));
        int ret = ::getsockopt(fd, level, flags, reinterpret_cast<detail::socket_traits::getopt_type>(&temp), &len);
        if(ret != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
        }
        return temp;
    }

    void binder(const char* str, unsigned port, std::error_code& ec) const noexcept {
        ec.clear();
        auto hints = addrinfo();
        std::unique_ptr<addrinfo, detail::addrinfo_deleter> res;

        hints.ai_family = protocol;
        hints.ai_socktype = type(ec);
        if(ec) {
            return; // abort
        }

        if(str == nullptr) {
            hints.ai_flags = AI_PASSIVE;
        }

        auto port_str = std::to_string(port);
        auto ptr = res.get();
        int ret = ::getaddrinfo(str, port_str.c_str(), &hints, &ptr);

        if(ret != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
            return; // can't bind yet
        }

        int error = ::bind(fd, ptr->ai_addr, ptr->ai_addrlen);

        if(error != 0) {
            ec.assign(error::get_last_error(), error::socket_category());
        }
    }

    native_type fd;
    int protocol;
};
} // net

#endif // LIBNET_SOCKET_HPP
