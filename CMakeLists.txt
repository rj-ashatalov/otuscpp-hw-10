cmake_minimum_required(VERSION 3.2)

function(add_executable_from_dir TARGET SRC_DIR)
    file(GLOB_RECURSE TARGET_SRC "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h")
    add_executable(${TARGET} ${TARGET_SRC})
endfunction()

project(bulk VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

configure_file(version.h.in ${PROJECT_SOURCE_DIR}/version.h)

add_executable_from_dir(${PROJECT_NAME} src)

add_executable(test_version tests/test_version.cpp src/Bulk.h)

add_library(patch_version lib.cpp)

set_target_properties(${PROJECT_NAME} patch_version test_version PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
)

set_target_properties(test_version PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    patch_version
    Threads::Threads
)

target_link_libraries(test_version
    ${Boost_LIBRARIES}
    patch_version
)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(test_version PUBLIC src)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT a.shatalov@rj-games.com)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

enable_testing()

add_test(${PROJECT_NAME}_tests test_version)

