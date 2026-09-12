add_executable(CjkStudyClockTest
  CjkStudyClockTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkStudyClock.cpp
)

target_include_directories(CjkStudyClockTest PRIVATE ${REPO_ROOT}/src)
target_link_libraries(CjkStudyClockTest PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(CjkStudyClockTest)
