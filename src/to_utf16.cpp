#include <cassert>

#include <Windows.h>

#include "to_utf16.hpp"

namespace epoll {
std::wstring toUtf16(std::string_view utf8)
{
  const std::size_t length{static_cast<std::size_t>(
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), -1, nullptr, 0))};
  std::wstring      result(length, L' ');

  const int statusCode{MultiByteToWideChar(
    CP_UTF8,
    0,
    utf8.data(),
    -1,
    result.data(),
    static_cast<int>(result.size()))};

  if (statusCode == 0) {
    assert(false && "Couldn't convert utf8 to utf16");
    return L"";
  }

  result.pop_back();
  return result;
}
} // namespace epoll
