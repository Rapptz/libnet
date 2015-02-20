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

#ifndef LIBNET_ERROR_HPP
#define LIBNET_ERROR_HPP

#include <net/detail/config.hpp>
#include <type_traits>
#include <system_error>
#include <string>
#include <cerrno>

#if defined(LIBNET_WINDOWS)
#include <windows.h>
#define LIBNET_ERROR(e) e
#define LIBNET_SOCKET_ERROR(e) WSA ## e
#define LIBNET_NETDB_ERROR(e) WSA ## e
#define LIBNET_GAI_ERROR(e) WSA ## e
#define LIBNET_POSIX_OR_WINDOWS(p, w) w
#else
#include <netdb.h>
#define LIBNET_ERROR(e) e
#define LIBNET_SOCKET_ERROR(e) e
#define LIBNET_NETDB_ERROR(e) e
#define LIBNET_GAI_ERROR(e) e
#define LIBNET_POSIX_OR_WINDOWS(p, w) p
#endif // LIBNET_WINDOWS

namespace net {
namespace error {
enum socket_error {
    access_denied                = LIBNET_SOCKET_ERROR(EACCES),
    address_family_not_supported = LIBNET_SOCKET_ERROR(EAFNOSUPPORT),
    address_in_use               = LIBNET_SOCKET_ERROR(EADDRINUSE),
    already_connected            = LIBNET_SOCKET_ERROR(EISCONN),
    already_started              = LIBNET_SOCKET_ERROR(EALREADY),
    bad_address                  = LIBNET_SOCKET_ERROR(EFAULT),
    bad_descriptor               = LIBNET_SOCKET_ERROR(EBADF),
    broken_pipe                  = LIBNET_POSIX_OR_WINDOWS(LIBNET_ERROR(EPIPE), LIBNET_ERROR(ERROR_BROKEN_PIPE)),
    connection_aborted           = LIBNET_SOCKET_ERROR(ECONNABORTED),
    connection_refused           = LIBNET_SOCKET_ERROR(ECONNREFUSED),
    connection_reset             = LIBNET_SOCKET_ERROR(ECONNRESET),
    connection_timed_out         = LIBNET_SOCKET_ERROR(ETIMEDOUT),
    host_unreachable             = LIBNET_SOCKET_ERROR(EHOSTUNREACH),
    in_progress                  = LIBNET_SOCKET_ERROR(EINPROGRESS),
    interrupted                  = LIBNET_SOCKET_ERROR(EINTR),
    invalid_argument             = LIBNET_SOCKET_ERROR(EINVAL),
    message_too_long             = LIBNET_SOCKET_ERROR(EMSGSIZE),
    name_too_long                = LIBNET_SOCKET_ERROR(ENAMETOOLONG),
    network_down                 = LIBNET_SOCKET_ERROR(ENETDOWN),
    network_reset                = LIBNET_SOCKET_ERROR(ENETRESET),
    network_unreachable          = LIBNET_SOCKET_ERROR(ENETUNREACH),
    no_buffer_space              = LIBNET_SOCKET_ERROR(ENOBUFS),
    no_descriptors               = LIBNET_SOCKET_ERROR(EMFILE),
    no_permission                = LIBNET_POSIX_OR_WINDOWS(LIBNET_ERROR(EPERM), LIBNET_ERROR(ERROR_ACCESS_DENIED)),
    not_connected                = LIBNET_SOCKET_ERROR(ENOTCONN),
    not_socket                   = LIBNET_SOCKET_ERROR(ENOTSOCK),
    operation_aborted            = LIBNET_POSIX_OR_WINDOWS(LIBNET_ERROR(ECANCELED), LIBNET_ERROR(ERROR_OPERATION_ABORTED)),
    operation_not_supported      = LIBNET_SOCKET_ERROR(EOPNOTSUPP),
    out_of_memory                = LIBNET_POSIX_OR_WINDOWS(LIBNET_ERROR(ENOMEM), LIBNET_ERROR(ERROR_OUTOFMEMORY)),
    protocol_unavailable         = LIBNET_SOCKET_ERROR(ENOPROTOOPT),
    shut_down                    = LIBNET_SOCKET_ERROR(ESHUTDOWN),
    try_again                    = LIBNET_POSIX_OR_WINDOWS(LIBNET_ERROR(EAGAIN), LIBNET_ERROR(ERROR_RETRY)),
    would_block                  = LIBNET_SOCKET_ERROR(EWOULDBLOCK)
};

enum gai_error {
    service_not_found       = LIBNET_POSIX_OR_WINDOWS(LIBNET_GAI_ERROR(EAI_SERVICE), LIBNET_ERROR(WSATYPE_NOT_FOUND)),
    socket_type_unsupported = LIBNET_POSIX_OR_WINDOWS(LIBNET_GAI_ERROR(EAI_SOCKTYPE), LIBNET_ERROR(WSAESOCKTNOSUPPORT))
};

enum netdb_error {
    host_not_found           = LIBNET_NETDB_ERROR(HOST_NOT_FOUND),
    host_not_found_try_again = LIBNET_NETDB_ERROR(TRY_AGAIN),
    no_data                  = LIBNET_NETDB_ERROR(NO_DATA),
    no_recovery              = LIBNET_NETDB_ERROR(NO_RECOVERY)
};

namespace detail {
// this is a work-around for MinGW's QoI with std::system_category.
struct socket_category : public std::error_category {
    const char* name() const noexcept override {
        return "libnet.socket";
    }

    std::string message(int value) const override {
        std::string buffer;
    #if defined(LIBNET_WINDOWS)
        buffer.resize(256);
        auto ret = ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    nullptr, // no format string provided
                                    value,   // the error code we want to turn to a string
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // default language
                                    &buffer[0],
                                    buffer.size(),
                                    nullptr); // no arguments for format string

        if(ret > 0) {
            buffer.resize(ret);
            // remove extraneous newlines
            while(buffer.back() == '\n' || buffer.back() == '\r') {
                buffer.pop_back();
            }
        }
        else {
            buffer = "Unknown error";
        }
    #else
        buffer = std::system_category().message(value);
    #endif // LIBNET_WINDOWS
        return buffer;
    }
};

struct netdb_category : public std::error_category {
    const char* name() const noexcept override {
        return "libnet.netdb";
    }

    std::string message(int value) const override {
        switch(value) {
        case net::error::host_not_found:
            return "Host not found";
        case net::error::host_not_found_try_again:
            return "Host not found, try again later";
        case net::error::no_data:
            return "Valid name but no data record found for the requested type";
        case net::error::no_recovery:
            return "A non-recoverable error has occurred during database lookup";
        default:
            return "An unknown error has occurred";
        }
    }
};

struct getaddrinfo_category : public std::error_category {
    const char* name() const noexcept override {
        return "libnet.getaddrinfo";
    }

    std::string message(int value) const override {
        switch(value) {
        case net::error::service_not_found:
            return "Service not found";
        case net::error::socket_type_unsupported:
            return "Socket type is unsupported";
        default:
            return "An unknown error has occurred";
        }
    }
};
} // detail

inline const std::error_category& getaddrinfo_category() {
    static detail::getaddrinfo_category instance;
    return instance;
}

inline const std::error_category& netdb_category() {
    static detail::netdb_category instance;
    return instance;
}


inline const std::error_category& socket_category() {
    static detail::socket_category instance;
    return instance;
}


inline std::error_code make_error_code(socket_error e) {
    return { static_cast<int>(e), socket_category() };
}

inline std::error_code make_error_code(gai_error e) {
    return { static_cast<int>(e), getaddrinfo_category() };
}

inline std::error_code make_error_code(netdb_error e) {
    return { static_cast<int>(e), netdb_category() };
}

inline std::error_condition make_error_condition(socket_error e) {
    return { static_cast<int>(e), socket_category() };
}

inline std::error_condition make_error_condition(gai_error e) {
    return { static_cast<int>(e), getaddrinfo_category() };
}

inline std::error_condition make_error_condition(netdb_error e) {
    return { static_cast<int>(e), netdb_category() };
}

inline int get_last_error() noexcept {
#if defined(LIBNET_WINDOWS)
    return WSAGetLastError();
#else
    return errno;
#endif // LIBNET_WINDOWS
}
} // error
} // net

namespace std {
template <>
struct is_error_code_enum<net::error::socket_error> : public true_type {};

template <>
struct is_error_code_enum<net::error::gai_error> : public true_type {};

template <>
struct is_error_code_enum<net::error::netdb_error> : public true_type {};
} // std

#undef LIBNET_ERROR
#undef LIBNET_SOCKET_ERROR
#undef LIBNET_NETDB_ERROR
#undef LIBNET_GAI_ERROR
#undef LIBNET_POSIX_OR_WINDOWS

#endif // LIBNET_ERROR_HPP
