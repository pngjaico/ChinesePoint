add_executable(study_state_tests
  StudyStateTest.cpp
  ${REPO_ROOT}/src/chinesepoint/study/StudyState.cpp
)
target_link_libraries(study_state_tests PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(study_state_tests)
