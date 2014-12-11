
cmake_minimum_required(VERSION 2.8.7)

project(disconet)

# Add extra warnings for GNU for debugging purposes
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/Modules)

option(ENABLE_PCAP "Find and use PCAP" ON)
if(ENABLE_PCAP)
  find_package(PCAP REQUIRED)
  include_directories(${PCAP_INCLUDE_DIRS})
  set(HAVE_PCAP True)
endif()

option(ENABLE_CURSES "Find and use curses" ON)
if(ENABLE_CURSES)
  find_package(Curses REQUIRED)
  include_directories(${CURSES_INCLUDE_DIRS})
  set(HAVE_CURSES True)
endif()

include_directories(${PROJECT_BINARY_DIR})  # For config.h
configure_file(config.h.in config.h)

set(disconet_SRCS
  disconet.cpp)

include(CheckIncludeFiles)
# Check for net/if_dl.h on bsd
check_include_files(net/if_dl.h HAVE_IFDL_H)

# Conditional source files

if(HAVE_IFDL_H)
  set(disconet_SRCS ${disconet_SRCS} bsd.cpp)
else()
  set(disconet_SRCS ${disconet_SRCS} linux.cpp)
endif()

if(CURSES_FOUND)
  set(disconet_SRCS ${disconet_SRCS} drawing.cpp)
endif()

if(PCAP_FOUND)
  set(disconet_SRCS ${disconet_SRCS} pcap.cpp)
endif()

add_executable(disconet ${disconet_SRCS})

# Add in the link libraries

if(PCAP_FOUND)
  target_link_libraries(disconet ${PCAP_LIBRARIES})
endif()

if(CURSES_FOUND)
  target_link_libraries(disconet ${CURSES_LIBRARIES})
endif()
