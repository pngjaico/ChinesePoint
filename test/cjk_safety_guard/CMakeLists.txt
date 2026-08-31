add_executable(CjkSafetyGuardTest
  CjkSafetyGuardTest.cpp
  ${REPO_ROOT}/src/chinesepoint/CjkSafetyGuard.cpp
)

target_include_directories(CjkSafetyGuardTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/src
)

target_compile_definitions(CjkSafetyGuardTest PRIVATE CHINESEPOINT=1)

target_link_libraries(CjkSafetyGuardTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkSafetyGuardTest)

add_executable(CjkSafetyGuardSimulatorTest
  CjkSafetyGuardSimulatorTest.cpp
  ${REPO_ROOT}/src/chinesepoint/CjkSafetyGuard.cpp
)

target_include_directories(CjkSafetyGuardSimulatorTest PRIVATE
  ${REPO_ROOT}/src
)

target_compile_definitions(CjkSafetyGuardSimulatorTest PRIVATE CHINESEPOINT=1 SIMULATOR=1)

target_link_libraries(CjkSafetyGuardSimulatorTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkSafetyGuardSimulatorTest)
