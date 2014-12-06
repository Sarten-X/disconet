find_path(PCAP_INCLUDE_DIR
  NAMES pcap.h
  PATH_SUFFIXES pcap)

find_library(PCAP_LIBRARY
  NAMES pcap)

# grep for version info (not most efficient...)
if(EXISTS "${PCAP_INCLUDE_DIR}/pcap.h")
  file(STRINGS "${PCAP_INCLUDE_DIR}/pcap.h" PCAP_STRINGS
    REGEX "^#define PCAP_VERSION_MAJOR")
  string(REGEX REPLACE "#define PCAP_VERSION_MAJOR +" "" PCAP_VERSION_MAJOR
    "${PCAP_STRINGS}")
  file(STRINGS "${PCAP_INCLUDE_DIR}/pcap.h" PCAP_STRINGS
    REGEX "^#define PCAP_VERSION_MINOR")
  string(REGEX REPLACE "#define PCAP_VERSION_MINOR +" "" PCAP_VERSION_MINOR
    "${PCAP_STRINGS}")
  set(PCAP_VERSION "${PCAP_VERSION_MAJOR}.${PCAP_VERSION_MINOR}")
endif()

set(PCAP_LIBRARIES ${PCAP_LIBRARY})
set(PCAP_INCLUDE_DIRS ${PCAP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PCAP
  REQUIRED_VARS PCAP_LIBRARIES PCAP_INCLUDE_DIRS
  VERSION_VAR PCAP_VERSION)

mark_as_advanced(PCAP_INCLUDE_DIRS PCAP_LIBRARIES)