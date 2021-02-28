cmake_minimum_required(VERSION 3.10)

project(RobotSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# used for the training
# add_executable(PracticeTests src/tests.cc)
# target_link_libraries(PracticeTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

include_directories(src)
add_library(util src/util/util.cc)
add_executable(util_test src/util/util_test.cc)
target_link_libraries(util_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} util)
