#include <cassert>

#include "format_error.hpp"

namespace epoll {
std::wstring formatError(DWORD errorCode)
{
  LPWSTR      buffer{nullptr};
  const DWORD statusCode{FormatMessageW(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER
      | FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr,
    errorCode,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(&buffer),
    0,
    nullptr)};
  assert((statusCode != 0) && "FormatMessageW failed!");

  if (buffer != nullptr) {
    const std::wstring errorMessage{buffer};
    const HLOCAL       hLocal{LocalFree(buffer)};
    assert((hLocal == nullptr) && "LocalFree failed!");
    buffer = nullptr;
    return errorMessage;
  }

  assert(false && "Could not allocate memory for FormatMessageW!");
  return L"";
}
} // namespace epoll
