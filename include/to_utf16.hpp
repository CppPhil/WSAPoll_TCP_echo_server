#pragma once
#include <string>
#include <string_view>

namespace epoll {
std::wstring toUtf16(std::string_view utf8);
} // namespace epoll
