# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.11)

message(STATUS "cmake home dir: [${CMAKE_HOME_DIRECTORY}].")
message(STATUS "cmake binary dir: [${CMAKE_BINARY_DIR}].")
message(STATUS "cmake source dir: [${CMAKE_SOURCE_DIR}].")
message(STATUS "cmake install profix dir: [${CMAKE_INSTALL_PREFIX}].")

project(main)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)



find_package(Boost COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARY_DIR})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
link_directories(${ZLIB_LIBRARY_DIR})

find_package(Threads REQUIRED)


include(ExternalProject)
ExternalProject_Add( uWS_p
  GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
  GIT_TAG master
  PREFIX ${CMAKE_HOME_DIRECTORY}/uWS
  UPDATE_COMMAND ""
  SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/uWS/source
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make -C ${CMAKE_HOME_DIRECTORY}/uWS/source &&
  cp -a ${CMAKE_HOME_DIRECTORY}/uWS/source/libuWS.so ${LIBRARY_OUTPUT_PATH}/  &&
  ln -s ${CMAKE_HOME_DIRECTORY}/uWS/source/src ${CMAKE_HOME_DIRECTORY}/include/uWS
  INSTALL_COMMAND  cmake -E echo "Skipping install step."
  # if you want to install
  # make -C ${CMAKE_HOME_DIRECTORY}/uWS/source install
  )


add_library(uWS SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(uWS PROPERTIES
  IMPORTED_LOCATION ${LIBRARY_OUTPUT_PATH}/libuWS.so
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_HOME_DIRECTORY}/uWS
)

add_dependencies(uWS uWS_p)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/include)

# file(GLOB src
#     "./include/*.h"
#     "./src/*.cpp"
# )

add_executable(main ${CMAKE_HOME_DIRECTORY}/src/main.cpp)
target_link_libraries( main uWS )
target_link_libraries( main ${Boost_LIBRARIES} )
target_link_libraries( main ${OPENSSL_LIBRARIES} )
target_link_libraries( main ${ZLIB_LIBRARIES} )
target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT})
add_executable(client ${CMAKE_HOME_DIRECTORY}/src/client.cpp)
target_link_libraries( client uWS )
target_link_libraries( client ${Boost_LIBRARIES} )
target_link_libraries( client ${OPENSSL_LIBRARIES} )
target_link_libraries( client ${ZLIB_LIBRARIES} )
target_link_libraries ( client ${CMAKE_THREAD_LIBS_INIT})
# install (TARGETS main DESTINATION bin)

