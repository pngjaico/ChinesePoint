add_executable(study_fsrs5_tests
  StudyFsrs5Test.cpp
  ${REPO_ROOT}/src/chinesepoint/study/StudyFsrs5.cpp
)
target_link_libraries(study_fsrs5_tests PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(study_fsrs5_tests)
