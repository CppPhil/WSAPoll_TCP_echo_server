#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>

#include "exception.hpp"
#include "finally.hpp"
#include "port.hpp"
#include "socket_library_manager.hpp"
#include "win_exception.hpp"

// epoll based Winsock TCP echo server
int main()
{
  try {
    epoll::SocketLibraryManager socketLibraryManager{};
    struct addrinfo             hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    struct addrinfo*  addrinfoList{nullptr};
    const std::string portText{std::to_string(epoll::port)};
    int               statusCode{
      getaddrinfo(nullptr, portText.c_str(), &hints, &addrinfoList)};

    if (statusCode != 0) {
      EPOLL_WIN_THROW(
        static_cast<DWORD>(statusCode), L"{}", L"getaddrinfo failed");
    }

    auto addrinfoListCleaner{epoll::finally([&addrinfoList] {
      if (addrinfoList != nullptr) {
        freeaddrinfo(addrinfoList);
      }
    })};

    SOCKET serverSocket{socket(
      addrinfoList->ai_family,
      addrinfoList->ai_socktype,
      addrinfoList->ai_protocol)};

    if (serverSocket == INVALID_SOCKET) {
      EPOLL_WIN_THROW(
        static_cast<DWORD>(WSAGetLastError()),
        L"{}",
        L"Could not create server socket");
    }

    auto serverSocketCloser{epoll::finally([&serverSocket] {
      closesocket(serverSocket);
      serverSocket = INVALID_SOCKET;
    })};

    statusCode = bind(
      serverSocket,
      addrinfoList->ai_addr,
      static_cast<int>(addrinfoList->ai_addrlen));

    if (statusCode == SOCKET_ERROR) {
      EPOLL_WIN_THROW(
        static_cast<DWORD>(WSAGetLastError()),
        L"{}",
        L"Could not bind server socket");
    }

    freeaddrinfo(addrinfoList);
    addrinfoList = nullptr;

    u_long enableNonBlockingMode{1};
    statusCode = ioctlsocket(serverSocket, FIONBIO, &enableNonBlockingMode);

    if (statusCode == SOCKET_ERROR) {
      EPOLL_WIN_THROW(
        static_cast<DWORD>(WSAGetLastError()),
        L"{}",
        L"Could not set non-blocking mode.");
    }

    statusCode = listen(serverSocket, SOMAXCONN);

    if (statusCode == SOCKET_ERROR) {
      EPOLL_WIN_THROW(
        static_cast<DWORD>(WSAGetLastError()),
        L"{}",
        L"Server socket failed to start listening");
    }

    std::cout << std::format("Listening on port {}.\n", epoll::port);

    std::vector<WSAPOLLFD> fdArray{};
    fdArray.push_back(WSAPOLLFD{
      .fd      = serverSocket,
      .events  = POLLIN,
      .revents = POLLERR | POLLHUP | POLLIN});
    auto fdArrayCleaner{epoll::finally([&fdArray, &serverSocket] {
      for (WSAPOLLFD& pollFd : fdArray) {
        if (pollFd.fd != serverSocket) {
          closesocket(pollFd.fd);
        }
      }
    })};
    std::unordered_map<SOCKET, std::string> addresses{};

    int pollResult{};

    while ((pollResult = WSAPoll(fdArray.data(), fdArray.size(), -1))
           != SOCKET_ERROR) {
      if (pollResult == 0) {
        continue;
      }

      int handledCount{0};

      for (std::size_t i{0};
           (i < fdArray.size()) && (handledCount != pollResult);
           ++i) {
        if ((fdArray[i].revents & (POLLERR | POLLHUP)) != 0) {
          std::cout << std::format(
            "\"{}\" disconnected.\n", addresses.find(fdArray[i].fd)->second);
          closesocket(fdArray[i].fd);
          fdArray.erase(fdArray.begin() + i);
          --i;
          ++handledCount;
        }
        else if ((fdArray[i].revents & POLLIN) != 0) {
          if (fdArray[i].fd == serverSocket) {
            // Accept incoming connection.
            sockaddr_in  inAddr{};
            int          addrLen{static_cast<int>(sizeof(inAddr))};
            const SOCKET clientSocket{accept(
              serverSocket, reinterpret_cast<sockaddr*>(&inAddr), &addrLen)};

            if (clientSocket == INVALID_SOCKET) {
              continue;
            }

            statusCode
              = ioctlsocket(clientSocket, FIONBIO, &enableNonBlockingMode);

            if (statusCode == SOCKET_ERROR) {
              EPOLL_WIN_THROW(
                static_cast<DWORD>(WSAGetLastError()),
                L"{}",
                L"Could not set non-blocking mode.");
            }

            fdArray.push_back(WSAPOLLFD{
              .fd      = clientSocket,
              .events  = POLLIN,
              .revents = POLLERR | POLLHUP | POLLIN});

            // We need at most 16 characters for IPv4 addresses.
            char addressBuffer[17]{};

            if (
              inet_ntop(AF_INET, &inAddr.sin_addr, addressBuffer, 16)
              == nullptr) {
              EPOLL_WIN_THROW(
                static_cast<DWORD>(WSAGetLastError()),
                L"{}",
                L"inet_ntop failed.");
            }

            addresses.emplace(clientSocket, addressBuffer);
            std::cout << std::format("\"{}\" connected.\n", addressBuffer);
          }
          else {
            // We have data coming in from a socket.
            constexpr std::size_t        bufferSize{512};
            std::array<char, bufferSize> buffer{};
            statusCode = recv(fdArray[i].fd, buffer.data(), buffer.size(), 0);

            if (statusCode == SOCKET_ERROR) {
              EPOLL_WIN_THROW(
                static_cast<DWORD>(WSAGetLastError()),
                L"{}",
                L"Failed to read data from client.");
            }

            const int bytesReceived{statusCode};

            // -1 to avoid printing the newline sent.
            std::cout << std::format(
              "Received \"{}\" from \"{}\"\n",
              std::string_view{
                buffer.data(), static_cast<std::size_t>(bytesReceived - 1)},
              addresses.find(fdArray[i].fd)->second);

            // Replace lowercase letters with uppercase letters.
            for (int j{0}; j < bytesReceived; ++j) {
              if (buffer[j] >= 'a' && buffer[j] <= 'z') {
                buffer[j] &= ~0x20;
              }
            }

            statusCode = send(fdArray[i].fd, buffer.data(), bytesReceived, 0);

            if (statusCode == SOCKET_ERROR) {
              EPOLL_WIN_THROW(
                static_cast<DWORD>(WSAGetLastError()),
                L"{}",
                L"Could not send data to client.");
            }

            std::cout << std::format(
              "Sent \"{}\" to \"{}\"\n",
              std::string_view{
                buffer.data(), static_cast<std::size_t>(bytesReceived - 1)},
              addresses.find(fdArray[i].fd)->second);
          }

          ++handledCount;
        }
      }
    }
  }
  catch (const epoll::WinException& winException) {
    const std::wstring exceptionText{winException.toString()};
    MessageBoxW(
      nullptr,
      exceptionText.c_str(),
      L"epoll::WinException caught",
      MB_ICONERROR | MB_OK);
  }
  catch (const epoll::Exception& exception) {
    const std::wstring exceptionText{exception.toString()};
    MessageBoxW(
      nullptr,
      exceptionText.c_str(),
      L"epoll::Exception caught",
      MB_ICONERROR | MB_OK);
  }
  catch (const std::exception& exception) {
    MessageBoxA(
      nullptr, exception.what(), "std::exception caught", MB_ICONERROR | MB_OK);
  }
  catch (...) {
    MessageBoxW(
      nullptr,
      L"Unknown exception caught!",
      L"Unknown exception caught",
      MB_ICONERROR | MB_OK);
  }
}
