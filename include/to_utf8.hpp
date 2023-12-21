#pragma once
#include <string>
#include <string_view>

namespace epoll {
std::string toUtf8(std::wstring_view utf16);
} // namespace epoll
