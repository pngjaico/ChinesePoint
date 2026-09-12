add_executable(study_revlog_tests
  StudyRevlogTest.cpp
  ${REPO_ROOT}/src/chinesepoint/study/StudyRevlog.cpp
)
target_link_libraries(study_revlog_tests PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(study_revlog_tests)
