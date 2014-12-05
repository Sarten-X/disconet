
cmake_minimum_required(VERSION 2.8.7)

project(disconet CXX)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIRS})

set(disconet_SRCS
  disconet.cpp bsd.cpp linux.cpp)

add_executable(disconet ${disconet_SRCS})
target_link_libraries(disconet ${CURSES_LIBRARIES})
