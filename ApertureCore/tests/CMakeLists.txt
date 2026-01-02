# Test configuration for ApertureCore

# Google Test - try to find, but don't fail if not found
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    message(STATUS "GTest not found, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

include(GoogleTest)

# Common test settings
set(TEST_LIBS aperturecore GTest::gtest GTest::gtest_main)

# Geometry tests
add_executable(geometry_tests
    geometry/PointTest.cpp
    geometry/BoundsTest.cpp
)
target_link_libraries(geometry_tests PRIVATE ${TEST_LIBS})
gtest_discover_tests(geometry_tests)

# Visibility tests (when implemented)
# add_executable(visibility_tests
#     visibility/TypeLimitsTest.cpp
# )
# target_link_libraries(visibility_tests PRIVATE ${TEST_LIBS})
# gtest_discover_tests(visibility_tests)

# Integration tests (when implemented)
# add_executable(integration_tests
#     integration/ClassicScenariosTest.cpp
# )
# target_link_libraries(integration_tests PRIVATE ${TEST_LIBS})
# gtest_discover_tests(integration_tests)
