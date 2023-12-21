#pragma once
#include <string>

#include <Windows.h>

namespace epoll {
std::wstring formatError(DWORD errorCode);
} // namespace epoll
