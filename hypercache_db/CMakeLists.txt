cmake_minimum_required(VERSION 3.28)
project(hc_db CXX)


find_package(Boost REQUIRED COMPONENTS system)

add_executable(hc_db src/hc_db.cpp src/main.cpp)

target_link_libraries(hc_db PRIVATE Boost::system)

install(TARGETS hc_db DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        )
