#pragma once

#define EPOLL_NONCOPYABLE(Class) \
  Class(const Class&) = delete;  \
  Class& operator=(const Class&) = delete
