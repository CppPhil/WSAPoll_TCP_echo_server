#include <ostream>
#include <utility>

#include "format_error.hpp"
#include "win_exception.hpp"

namespace epoll {
WinException::WinException(
  std::wstring file,
  std::wstring function,
  std::size_t  line,
  std::wstring message,
  DWORD        errorCode)
  : Exception{std::move(file), std::move(function), line, std::move(message)}
  , m_errorCode{errorCode}
  , m_winErrorMessage{formatError(m_errorCode)}
{
}

void WinException::printOn(std::wostream& wos) const
{
  wos << L"WinException{\n"
      << L"  file                 : \"" << file() << L"\"\n"
      << L"  function             : \"" << function() << L"\"\n"
      << L"  line                 :  " << line() << L"\n"
      << L"  Windows error message: \"" << m_winErrorMessage << L"\"\n"
      << L"  message              : \"" << wideWhat() << L'"';
}

DWORD WinException::errorCode() const noexcept
{
  return m_errorCode;
}
} // namespace epoll
