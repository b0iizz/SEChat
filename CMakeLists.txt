cmake_minimum_required(VERSION 3.10)
project(SEChat)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(tidy "Use clang-tidy to improve code style in the project" OFF)

if(tidy AND UNIX)
  set (CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES Off)
  set (CMAKE_C_CLANG_TIDY clang-tidy --config-file=./.clang-tidy )
endif()

set(sources
  src/main.c
  src/interface.c
  src/encrypt.c
  src/packet.c
)

if(WIN32)
  set(platform_sources
    src/windows/terminal.c
    src/windows/socket.c
  )
elseif(UNIX)
  set(platform_sources
    src/unix/socket.c
    src/unix/terminal.c
  )
endif()

add_executable(sechat ${sources} ${platform_sources})

target_compile_options(sechat PUBLIC -Wall -Wextra -pedantic -O2)
target_include_directories(sechat PUBLIC src)

if(WIN32)
  target_link_libraries(sechat wsock32 ws2_32)
  target_link_libraries(sechat -static)
endif()

install(TARGETS sechat DESTINATION bin)
