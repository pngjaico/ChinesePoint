add_executable(CjkLearnerStatsTest
  CjkLearnerStatsTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkLearnerStats.cpp
)

target_include_directories(CjkLearnerStatsTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkLearnerStatsTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkLearnerStatsTest)
