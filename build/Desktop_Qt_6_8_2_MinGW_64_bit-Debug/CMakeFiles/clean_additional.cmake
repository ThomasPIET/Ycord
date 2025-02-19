# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\YTsap_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\YTsap_autogen.dir\\ParseCache.txt"
  "YTsap_autogen"
  )
endif()
