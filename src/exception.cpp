#include <locale>
#include <ostream>
#include <sstream>
#include <utility>

#include "exception.hpp"
#include "to_utf8.hpp"

namespace epoll {
std::wostream& operator<<(std::wostream& wos, const Exception& exception)
{
  exception.printOn(wos);
  return wos;
}

Exception::Exception(
  std::wstring file,
  std::wstring function,
  std::size_t  line,
  std::wstring message)
  : m_file{std::move(file)}
  , m_function{std::move(function)}
  , m_line{line}
  , m_message{std::move(message)}
  , m_utf8Message{toUtf8(m_message)}
{
}

void Exception::printOn(std::wostream& wos) const
{
  wos << L"Exception{\n"
      << L"  file    : \"" << file() << L"\"\n"
      << L"  function: \"" << function() << L"\"\n"
      << L"  line    :  " << line() << L"\n"
      << L"  message : \"" << wideWhat() << L'"';
}

const wchar_t* Exception::wideWhat() const noexcept
{
  return m_message.c_str();
}

const char* Exception::what() const noexcept
{
  return m_utf8Message.c_str();
}

const std::wstring& Exception::file() const noexcept
{
  return m_file;
}

const std::wstring& Exception::function() const noexcept
{
  return m_function;
}

std::size_t Exception::line() const noexcept
{
  return m_line;
}

std::wstring Exception::toString() const
{
  std::wostringstream woss{};
  woss.imbue(std::locale::classic());
  woss << *this;
  return woss.str();
}
} // namespace epoll
