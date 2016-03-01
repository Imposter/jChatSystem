/*
*   This file is part of the jChatSystem project.
*
*   This program is licensed under the GNU General
*   Public License. To view the full license, check
*   LICENSE in the project root.
*/

#ifndef jchat_lib_tcp_client_hpp_
#define jchat_lib_tcp_client_hpp_

// Required libraries
#include "platform.h"
#include "event.hpp"
#include "buffer.hpp"
#include <mutex>
#include <thread>
#if defined(OS_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef __SOCKET__
#define __SOCKET__
typedef int SOCKET;
#endif // __SOCKET__

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif // SOCKET_ERROR

#ifndef __CLOSE_SOCKET__
#define __CLOSE_SOCKET__
#define closesocket(socket_fd) close(socket_fd)
#endif // __CLOSE_SOCKET__
#elif defined(OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#endif

// NOTE: DEBUG!
#include <stdio.h>

namespace jchat {
class TcpClient {
  const char *hostname_;
  uint16_t port_;
  bool is_blocking_;
  bool is_connected_;
  bool is_internal_;
  SOCKET client_socket_;
  sockaddr_in client_endpoint_;
  std::thread worker_thread_;

  void worker_loop() {
    while (is_connected_) {
      // TODO: Write!

    }
  }

public:
  TcpClient(const char *hostname, uint16_t port, bool is_blocking = true)
    : hostname_(hostname), port_(port), is_blocking_(is_blocking),
    client_socket_(0), is_connected_(false), is_internal_(false) {
    client_endpoint_.sin_family = AF_INET;
    client_endpoint_.sin_port = htons(port);
#if defined(OS_LINUX)
    client_endpoint_.sin_addr.s_addr = inet_addr("127.0.0.1");
#elif defined(OS_WIN) // Currently untested
    client_endpoint_.sin_addr.S_addr = inet_addr("127.0.0.1");
#endif

    // Get remote address info
    addrinfo *result = nullptr;
    addrinfo hints;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int return_value = getaddrinfo(hostname, std::to_string(port).c_str(),
      &hints, &result);
    if (return_value != SOCKET_ERROR) {
      for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
          sockaddr_in *endpoint_info = (sockaddr_in *)ptr->ai_addr;
#if defined(OS_LINUX)
          client_endpoint_.sin_addr.s_addr = endpoint_info->sin_addr.s_addr;
#elif defined(OS_WIN) // Currently untested
          client_endpoint_.sin_addr.S_un.S_addr
            = endpoint_info->sin_addr.S_un.S_addr;
#endif
        }
      }
    }

     freeaddrinfo(result);
  }

  // NOTE: For internal usage only!
  TcpClient(SOCKET client_socket, sockaddr_in remote_endpoint,
    bool is_blocking = true)
    : client_socket_(client_socket), client_endpoint_(remote_endpoint),
    is_blocking_(is_blocking), is_connected_(true), is_internal_(true) {
    if (!is_blocking) {
#if defined(OS_LINUX)
      uint32_t flags = fcntl(client_socket, F_GETFL, 0);
      if (flags != SOCKET_ERROR) {
        flags &= ~O_NONBLOCK;
        fcntl(client_socket, F_SETFL, flags);
      }
#elif defined(OS_WIN)
      uint32_t blocking = is_blocking ? 0 : 1;
      ioctlsocket(client_socket, FIONBIO, &blocking);
#endif
    }
  }

  ~TcpClient() {
    if (is_connected_) {
      is_connected_ = false;
      if (!is_internal_) {
        worker_thread_.join();
      }
      closesocket(client_socket_);
#if defined(OS_WIN)
      WSACleanup();
#endif
    }
  }

  bool Connect() {
    if (is_connected_ || is_internal_) {
      return false;
    }

#if defined(OS_WIN)
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
      return false;
    }
#endif

    if ((client_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
      == SOCKET_ERROR) {
      return false;
    }

    if (connect(client_socket_, (const sockaddr *)&client_endpoint_,
      sizeof(client_endpoint_)) == SOCKET_ERROR) {
      closesocket(client_socket_);
      return false;
    }

    if (!is_blocking_) {
#if defined(OS_LINUX)
      uint32_t flags = fcntl(client_socket_, F_GETFL, 0);
      if (flags != SOCKET_ERROR) {
        flags &= ~O_NONBLOCK;
        if (fcntl(client_socket_, F_SETFL, flags) == SOCKET_ERROR) {
          closesocket(client_socket_);
          return false;
        }
      } else {
        closesocket(client_socket_);
        return false;
      }
#elif defined(OS_WIN)
      uint32_t blocking = is_blocking ? 0 : 1;
      if (ioctlsocket(client_socket_, FIONBIO, &blocking) == SOCKET_ERROR) {
        closesocket(client_socket_);
        return false;
      }
#endif
    }

    worker_thread_ = std::thread(&TcpClient::worker_loop, this);

    is_connected_ = true;
    OnConnected();

    return true;
  }

  bool Disconnect() {
    if (!is_connected_) {
      return false;
    }

    is_connected_ = false;
    if (!is_internal_) {
      worker_thread_.join();
    }
    closesocket(client_socket_);

#if defined(OS_WIN)
    WSACleanup();
#endif

    OnDisconnected();

    return true;
  }

  // NOTE: These are not needed for internal usage!
  Event<> OnConnected;
  Event<> OnDisconnected;
  Event<Buffer &> OnDataReceived;
};
}

#endif // jchat_lib_tcp_client_hpp_
