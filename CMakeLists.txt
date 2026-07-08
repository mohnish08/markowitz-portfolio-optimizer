cmake_minimum_required(VERSION 3.16)
project(PortfolioOptimizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -O2)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(portfolio_core
    src/Matrix.cpp
    src/CsvReader.cpp
    src/ReturnsCalculator.cpp
    src/CovarianceMatrix.cpp
    src/PortfolioMetrics.cpp
    src/MarkowitzOptimizer.cpp
    src/EfficientFrontier.cpp
)

target_include_directories(portfolio_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(portfolio_optimizer src/main.cpp)
target_link_libraries(portfolio_optimizer PRIVATE portfolio_core)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
