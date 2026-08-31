add_executable(CjkJournalTest
  CjkJournalTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkJournal.cpp
)

target_include_directories(CjkJournalTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkJournalTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkJournalTest)
