cmake_minimum_required(VERSION 3.5)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})


project(bayan VERSION ${PROJECT_VERSION})

find_package(Boost COMPONENTS filesystem program_options unit_test_framework REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

add_executable(bayan bayan.cpp)

target_link_libraries(bayan
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
