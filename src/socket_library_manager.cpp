#include "socket_library_manager.hpp"
#include "win_exception.hpp"

namespace epoll {
constexpr BYTE majorVersion{2};
constexpr BYTE minorVersion{2};
constexpr WORD requestedVersion{MAKEWORD(majorVersion, minorVersion)};

SocketLibraryManager::SocketLibraryManager() : m_wsaData{}
{
  const int statusCode{WSAStartup(requestedVersion, &m_wsaData)};

  if (statusCode != 0) {
    EPOLL_WIN_THROW(
      static_cast<DWORD>(statusCode), L"{}", L"WSAStartup failed");
  }

  if (
    LOBYTE(m_wsaData.wVersion) != majorVersion
    || HIBYTE(m_wsaData.wVersion) != minorVersion) {
    WSACleanup();
    EPOLL_THROW(L"Could not find a usable version of Winsock.dll");
  }
}

SocketLibraryManager::~SocketLibraryManager()
{
  WSACleanup();
}
} // namespace epoll
