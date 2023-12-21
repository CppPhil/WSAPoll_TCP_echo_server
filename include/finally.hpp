#pragma once
#include <type_traits>
#include <utility>

#include "noncopyable.hpp"

namespace epoll {
template<typename Callable>
class FinalAct {
public:
  EPOLL_NONCOPYABLE(FinalAct);

  explicit FinalAct(const Callable& callable) noexcept
    : m_callable{callable}, m_shouldCall{true}
  {
  }

  explicit FinalAct(Callable&& callable) noexcept
    : m_callable{std::move(callable)}, m_shouldCall{true}
  {
  }

  FinalAct(FinalAct&& other) noexcept
    : m_callable{std::move(other.m_callable)}
    , m_shouldCall{std::exchange(other.m_shouldCall, false)}
  {
  }

  FinalAct& operator=(FinalAct&&) = delete;

  ~FinalAct()
  {
    if (m_shouldCall) {
      m_callable();
    }
  }

private:
  Callable m_callable;
  bool     m_shouldCall;
};

template<typename Callable>
FinalAct<std::remove_cv_t<std::remove_reference_t<Callable>>> finally(
  Callable&& callable) noexcept
{
  return FinalAct<std::remove_cv_t<std::remove_reference_t<Callable>>>{
    std::forward<Callable>(callable)};
}
} // namespace epoll
