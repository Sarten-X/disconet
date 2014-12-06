
cmake_minimum_required(VERSION 2.8.7)

project(disconet)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIRS})


set(disconet_SRCS
  disconet.cpp drawing.cpp)

include(CheckIncludeFiles)
# Check for net/if_dl.h on bsd
check_include_files(net/if_dl.h HAVE_IFDL_H)

if(HAVE_IFDL_H)
  set(disconet_SRCS ${disconet_SRCS} bsd.cpp)
else()
  set(disconet_SRCS ${disconet_SRCS} linux.cpp)
endif()


add_executable(disconet ${disconet_SRCS})
target_link_libraries(disconet ${CURSES_LIBRARIES})
