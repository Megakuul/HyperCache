cmake_minimum_required(VERSION 3.28)
project(hc_proxy)

set(CMAKE_CXX_STANDARD 23)

if(STATIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE
  src/main.cc
)

set(BOOST_COMPONENTS
  system
  filesystem
)

find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})

include_directories(includes)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} Boost::${BOOST_COMPONENTS})
