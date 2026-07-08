include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(TEST_SOURCES
    test_matrix.cpp
    test_csv_reader.cpp
    test_returns_calculator.cpp
    test_covariance_matrix.cpp
    test_portfolio_metrics.cpp
    test_markowitz_optimizer.cpp
    test_efficient_frontier.cpp
)

add_executable(portfolio_tests ${TEST_SOURCES})
target_link_libraries(portfolio_tests PRIVATE portfolio_core gtest gtest_main)
target_include_directories(portfolio_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(portfolio_tests)
