cmake_minimum_required(VERSION 3.10)
project(SEChat)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_COMPILER gcc)

set(sources
  src/main.c
)

if(WIN32)
  set(platform_sources
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
endif()

install(TARGETS sechat DESTINATION bin)
