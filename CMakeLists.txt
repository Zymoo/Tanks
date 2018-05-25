# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.11)

message(STATUS "cmake home dir: [${CMAKE_HOME_DIRECTORY}].")
message(STATUS "cmake bianry dir: [${CMAKE_BINARY_DIR}].")
message(STATUS "cmake source dir: [${CMAKE_SOURCE_DIR}].")
message(STATUS "cmake install profix dir: [${CMAKE_INSTALL_PREFIX}].")

project(main)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR/lib})



find_package(Boost COMPONENTS filesystem  REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARY_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARY_DIR})

find_package(Threads REQUIRED)

add_library(uWS SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(uWS PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/uWebSockets/libuWS.so"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/uWebSockets/src"
)


file(GLOB src
    "./include/*.h"
    "./src/*.cpp"
)

add_executable(main ${src})
target_link_libraries( main uWS )
target_link_libraries( main ${Boost_LIBRARIES} )
target_link_libraries( main ${OPENSSL_LIBRARIES} )
target_link_libraries( main ${ZLIB_LIBRARIES} )
target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT})
install (TARGETS main DESTINATION bin)
