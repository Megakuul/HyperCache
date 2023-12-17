cmake_minimum_required(VERSION 3.15)
project(hypercache_proxy)

set(CMAKE_CXX_STANDARD 23)

if(STATIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE
  src/main.cc
)

include_directories(includes)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
