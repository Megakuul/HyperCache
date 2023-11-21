cmake_minimum_required(VERSION 3.15)
project(hypercache_proxy)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE
  src/main.cc
)

if(STATIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(hp_prox src/main.cpp)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")


include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(hc_prox src/main.cpp)
target_link_libraries(hp_prox ${CONAN_LIBS})
