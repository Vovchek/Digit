# Test configuration for FringesCore

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
set(TEST_LIBS fringescore GTest::gtest GTest::gtest_main)

# fringe core tests
add_executable(fringesegments_tests
    FringeSegmentTest.cpp
)
target_link_libraries(fringesegments_tests PRIVATE ${TEST_LIBS})
gtest_discover_tests(fringesegments_tests)

# auto-numbering tests
add_executable(autonumbering_tests
    AutoNumberingTest.cpp
)
target_link_libraries(autonumbering_tests PRIVATE ${TEST_LIBS})
gtest_discover_tests(autonumbering_tests)

# Interpolation tests
add_executable(interpolation_tests
    InterpolationTest.cpp
)
target_link_libraries(interpolation_tests PRIVATE ${TEST_LIBS})
gtest_discover_tests(interpolation_tests)

# Integration tests (when implemented)
# add_executable(integration_tests
#     integration/ClassicScenariosTest.cpp
# )
# target_link_libraries(integration_tests PRIVATE ${TEST_LIBS})
# gtest_discover_tests(integration_tests)
