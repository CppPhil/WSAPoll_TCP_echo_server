cmake_minimum_required(VERSION 3.20)

project(epoll_server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
  server
  include/exception.hpp
  include/finally.hpp
  include/format_error.hpp
  include/noncopyable.hpp
  include/port.hpp
  include/socket_library_manager.hpp
  include/to_utf16.hpp
  include/to_utf8.hpp
  include/win_exception.hpp
  src/exception.cpp
  src/format_error.cpp
  src/main.cpp
  src/socket_library_manager.cpp
  src/to_utf16.cpp
  src/to_utf8.cpp
  src/win_exception.cpp
)

target_include_directories(
  server
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
  target_link_libraries(server PRIVATE wsock32 ws2_32)
  target_compile_definitions(server PRIVATE WIN32_LEAN_AND_MEAN)
endif()
