cmake_minimum_required(VERSION 3.8)
project(html2md VERSION 1.5.3 LANGUAGES CXX)

set(PROJECT_HOMEPAGE_URL "https://tim-gromeyer.github.io/html2md/")
set(html2md_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")

set(PROJECT_DESCRIPTION "Transform your HTML into clean, easy-to-read markdown with html2md")
set(html2md_DESCRIPTION "${PROJECT_DESCRIPTION}")

# If build type not specified we use release
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified. Release is used.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Improve performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Check if it was included via `add_subdirectory`
get_directory_property(subproject PARENT_DIRECTORY)

# Create HTML for webassembly
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Some options
if (subproject)
    option(BUILD_EXE "Build a executable to convert html to markdown." OFF)
else()
    option(BUILD_EXE "Build a executable to convert html to markdown." ON)
endif()
option(BUILD_DOC "Build documentation" OFF)
option(BUILD_TEST "Build tests" OFF)
option(PYTHON_BINDINGS "Build python bindings" OFF)

set(SOURCES
    src/html2md.cpp
    src/table.cpp
)
set(HEADERS
    include/html2md.h
    include/table.h
)

if(PYTHON_BINDINGS)
    add_subdirectory(python/pybind11)
    pybind11_add_module(pyhtml2md python/bindings.cpp ${SOURCES} ${HEADER})
    target_compile_features(pyhtml2md PUBLIC
        cxx_auto_type # auto keyword
        cxx_constexpr # constexpr support
        cxx_range_for # for (auto test : tests)
        cxx_std_11 # Require at least c++11
    )
    target_compile_definitions(pyhtml2md PRIVATE PYTHON_BINDINGS)
    target_include_directories(pyhtml2md PRIVATE include)
    return()
endif()

add_library(html2md ${SOURCES})
set_target_properties(html2md PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${HEADERS}"
)
target_include_directories(html2md PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(html2md PUBLIC cxx_std_11) # Require at least c++11

if ((subproject AND BUILD_SHARED_LIBS) OR BUILD_EXE)
    add_library(html2md-static STATIC ${HEADERS} ${SOURCES})
    target_include_directories(html2md-static PUBLIC include)
    target_compile_features(html2md-static PUBLIC cxx_std_11) # Require at least c++11
endif()

if(BUILD_EXE)
    add_executable(html2md-exe cli/main.cpp)
    target_link_libraries(html2md-exe html2md-static)
    set_target_properties(html2md-exe PROPERTIES OUTPUT_NAME "html2md")
    target_compile_definitions(html2md-exe PUBLIC VERSION="${PROJECT_VERSION}")
    target_compile_features(html2md-exe PUBLIC cxx_std_11) # Require at least c++11
endif()

if(BUILD_TEST)
    add_subdirectory(tests)
endif()

if(BUILD_DOC)
    include(cmake/Doc.cmake)
endif()

# Don't install as a subproject
if(subproject)
    return()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS html2md
    EXPORT html2mdTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/html2md
)
install(EXPORT html2mdTargets
        FILE html2mdTargets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/html2md"
)

configure_file(html2md.pc.in html2md.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/html2md.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/html2md
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/html2md
)

if (BUILD_EXE)
    install(TARGETS html2md-exe DESTINATION bin)
endif()

include(cmake/Packaging.cmake)
