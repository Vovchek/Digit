cmake_minimum_required(VERSION 3.15)
project(ApertureCore VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(APERTURE_BUILD_TESTS "Build tests" ON)
option(APERTURE_BUILD_LEGACY "Build legacy compatibility layer" ON)
option(APERTURE_BUILD_EXAMPLES "Build examples" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Library sources
set(APERTURE_SOURCES
    # Geometry (only implemented so far)
    src/geometry/Point.cpp
    src/geometry/Bounds.cpp
    
    # Visibility (only TypeLimits so far)
    src/visibility/TypeLimits.cpp
    
    # TODO: Add as implemented
    # src/geometry/Shape.cpp
    # src/geometry/Ellipse.cpp
    # src/geometry/Rectangle.cpp
    # src/geometry/Polygon.cpp
    # src/geometry/BrokenLine.cpp
    # src/visibility/ShapeCollection.cpp
    # src/visibility/VisibilityChecker.cpp
    # src/visibility/VisibilityMask.cpp
    # src/contour/ContourExtractor.cpp
    # src/contour/SegmentConnector.cpp
    # src/io/ShapeSerializer.cpp
    # src/io/BoundsIO.cpp
)

# Main library
add_library(aperturecore ${APERTURE_SOURCES})
add_library(ApertureCore::Core ALIAS aperturecore)

target_include_directories(aperturecore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(aperturecore PRIVATE APERTURE_EXPORTS)
endif()

# Legacy compatibility layer
if(APERTURE_BUILD_LEGACY)
    # TODO: Implement legacy adapter
    # add_library(aperturecore_legacy
    #     src/legacy/LegacyAdapter.cpp
    #     src/legacy/XYShapeAdapter.cpp
    # )
    # add_library(ApertureCore::Legacy ALIAS aperturecore_legacy)
    # 
    # target_link_libraries(aperturecore_legacy PUBLIC aperturecore)
    # target_include_directories(aperturecore_legacy
    #     PUBLIC
    #         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #         $<INSTALL_INTERFACE:include>
    # )
endif()

# Tests
if(APERTURE_BUILD_TESTS)
    enable_testing()
    
    # Find or fetch Google Test
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    
    add_subdirectory(tests)
endif()

# Examples
if(APERTURE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS aperturecore
    EXPORT ApertureCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/aperturecore
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT ApertureCoreTargets
    FILE ApertureCoreTargets.cmake
    NAMESPACE ApertureCore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ApertureCore
)

# Package config
configure_package_config_file(
    cmake/ApertureCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ApertureCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ApertureCore
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ApertureCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ApertureCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ApertureCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ApertureCore
)
