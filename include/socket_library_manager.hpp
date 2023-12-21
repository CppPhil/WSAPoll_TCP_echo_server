#pragma once
#include <WinSock2.h>

#include "noncopyable.hpp"

namespace epoll {
class SocketLibraryManager {
public:
  EPOLL_NONCOPYABLE(SocketLibraryManager);

  SocketLibraryManager();

  ~SocketLibraryManager();

private:
  WSADATA m_wsaData;
};
} // namespace epoll
