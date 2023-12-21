#include <cassert>

#include <Windows.h>

#include "to_utf8.hpp"

namespace epoll {
std::string toUtf8(std::wstring_view utf16)
{
  const std::size_t bytesNeeded{static_cast<std::size_t>(WideCharToMultiByte(
    CP_UTF8, 0, utf16.data(), -1, nullptr, 0, nullptr, nullptr))};
  std::string       result(bytesNeeded, ' ');

  const int statusCode{WideCharToMultiByte(
    CP_UTF8,
    0,
    utf16.data(),
    -1,
    result.data(),
    static_cast<int>(result.size()),
    nullptr,
    nullptr)};

  if (statusCode == 0) {
    assert(false && "Couldn't convert utf16 to utf8");
    return "";
  }

  result.pop_back();
  return result;
}
} // namespace epoll
