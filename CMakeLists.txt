cmake_minimum_required(VERSION 3.15)
project(bayan)

set(CMAKE_CXX_STANDARD 17)


cmake_policy(SET CMP0167 NEW)


find_package(Boost 1.65 REQUIRED COMPONENTS filesystem program_options unit_test_framework)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(bayan bayan.cpp)
  
    target_link_libraries(bayan Boost::filesystem Boost::program_options Boost::unit_test_framework)
else()
    message(FATAL_ERROR "Boost not found")
endif()
