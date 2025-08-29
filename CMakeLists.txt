cmake_minimum_required(VERSION 3.15)
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})


project(bayan VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 17)


if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()


find_package(Boost 1.70 CONFIG REQUIRED COMPONENTS filesystem program_options unit_test_framework)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(bayan bayan.cpp)
    target_link_libraries(bayan Boost::filesystem Boost::program_options Boost::unit_test_framework)
else()
    message(FATAL_ERROR "Boost not found")
endif()
install(TARGETS print_ip RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
