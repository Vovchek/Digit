cmake_minimum_required(VERSION 3.15)
project(FringesCore VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(FRINGES_BUILD_TESTS "Build tests" ON)
option(FRINGES_BUILD_LEGACY "Build legacy compatibility layer" ON)
option(FRINGES_BUILD_EXAMPLES "Build examples" OFF)
option(FRINGES_BUILD_DOCS "Build documentation" ON)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Library sources
set(FRINGES_SOURCES
    src/FringeSegment.cpp
    src/WavefrontFromContours.cpp
)

# Main library
add_library(fringescore ${FRINGES_SOURCES})
add_library(FringesCore::Core ALIAS fringescore)

# ApertureCore is a sibling source dependency when building the repository tree.
set(APERTURECORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ApertureCore" CACHE PATH
    "Path to the ApertureCore source directory")

if(NOT EXISTS "${APERTURECORE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "ApertureCore was not found at '${APERTURECORE_SOURCE_DIR}'. "
        "Set APERTURECORE_SOURCE_DIR to its source directory.")
endif()

add_subdirectory(
    "${APERTURECORE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/ApertureCore"
)

target_include_directories(fringescore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

    target_link_libraries(fringescore
        PUBLIC
        ApertureCore::Core
    )

# Platform-specific settings
if(WIN32)
    target_compile_definitions(fringescore PRIVATE FRINGES_EXPORTS)
endif()

# Legacy compatibility layer
if(FRINGES_BUILD_LEGACY)
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
if(FRINGES_BUILD_TESTS)
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
if(FRINGES_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Documentation
if(FRINGES_BUILD_DOCS)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")
        
        # Check for optional components separately (don't fail if missing)
        find_program(DOXYGEN_DOT_EXECUTABLE
            NAMES dot
            DOC "Graphviz Dot tool for generating diagrams"
        )
        
        if(DOXYGEN_DOT_EXECUTABLE)
            message(STATUS "Graphviz dot found: ${DOXYGEN_DOT_EXECUTABLE}")
            set(DOXYGEN_HAVE_DOT "YES")
        else()
            message(STATUS "Graphviz dot not found - diagrams will be disabled")
            set(DOXYGEN_HAVE_DOT "NO")
        endif()
        
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        
        # Configure Doxyfile (replaces @DOXYGEN_HAVE_DOT@)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(fringescore-docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        
        message(STATUS "Documentation target 'fringescore-docs' created")
        message(STATUS "  Build with: cmake --build build --target fringescore-docs")
    else()
        message(WARNING "Doxygen not found - documentation will not be built")
        message(WARNING "  Install Doxygen to enable documentation generation")
    endif()
endif()

# Install rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS fringescore
    EXPORT FringesCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/fringescore
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT FringesCoreTargets
    FILE FringesCoreTargets.cmake
    NAMESPACE fringescore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FringesCore
)

# Package config
configure_package_config_file(
    cmake/FringesCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/FringesCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FringesCore
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/FringesCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/FringesCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/FringesCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FringesCore
)
