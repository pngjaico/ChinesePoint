add_executable(CjkReviewSchedulerTest
  CjkReviewSchedulerTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkReviewScheduler.cpp
)

target_include_directories(CjkReviewSchedulerTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkReviewSchedulerTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkReviewSchedulerTest)
