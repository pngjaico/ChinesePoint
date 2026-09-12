add_executable(study_cards_tests
  StudyCardsTest.cpp
  ${REPO_ROOT}/src/chinesepoint/study/StudyCards.cpp
)
target_link_libraries(study_cards_tests PRIVATE crosspoint_test_common GTest::gtest_main)
gtest_discover_tests(study_cards_tests)
