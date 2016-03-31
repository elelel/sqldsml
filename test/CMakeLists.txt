cmake_minimum_required(VERSION 3.4.0)
project(sqldsml_test)

set(CMAKE_CXX_STANDARD 11)

enable_testing()
find_package(GTest REQUIRED)

if (NOT WIN32)
  set(LINUX_LIBS "${CMAKE_DL_LIBS}")
endif()

#FIX these paths according to your locations
set(SQLITE_HPP_PATH "lib/sqlite-hpp")

add_subdirectory("${SQLITE_HPP_PATH}/test/lib/sqlite" "${CMAKE_CURRENT_BINARY_DIR}/sqlite3")
include_directories("${SQLITE_HPP_PATH}/test/lib/sqlite/src")
include_directories("${SQLITE_HPP_PATH}/include")

include_directories(${GTEST_INCLUDE_DIRES})
include_directories("../include")
add_executable(sqldsml_test src/sqldsml_test.cpp)
target_link_libraries(sqldsml_test ${GTEST_BOTH_LIBRARIES} ${LINUX_LIBS} sqlite3 pthread)
add_test(SqldsmlTests sqldsml_test)

