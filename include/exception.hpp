#pragma once
#include <cstddef>

#include <exception>
#include <format>
#include <iosfwd>
#include <string>

#include "noncopyable.hpp"
#include "to_utf16.hpp"

namespace epoll {
#define EPOLL_THROW(FormatString, ...)                         \
  throw ::epoll::Exception                                     \
  {                                                            \
    ::epoll::toUtf16(__FILE__), ::epoll::toUtf16(__FUNCSIG__), \
      static_cast<std::size_t>(__LINE__),                      \
      std::format(FormatString, __VA_ARGS__)                   \
  }

class Exception : public std::exception {
public:
  EPOLL_NONCOPYABLE(Exception);

  friend std::wostream& operator<<(
    std::wostream&   wos,
    const Exception& exception);

  Exception(
    std::wstring file,
    std::wstring function,
    std::size_t  line,
    std::wstring message);

  virtual void printOn(std::wostream& wos) const;

  const wchar_t* wideWhat() const noexcept;

  const char* what() const noexcept override;

  const std::wstring& file() const noexcept;

  const std::wstring& function() const noexcept;

  std::size_t line() const noexcept;

  std::wstring toString() const;

private:
  std::wstring m_file;
  std::wstring m_function;
  std::size_t  m_line;
  std::wstring m_message;
  std::string  m_utf8Message;
};
} // namespace epoll
