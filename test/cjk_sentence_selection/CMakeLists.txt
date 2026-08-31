add_executable(CjkSentenceSelectionTest
  CjkSentenceSelectionTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkSentenceSelection.cpp
)

target_include_directories(CjkSentenceSelectionTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkSentenceSelectionTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkSentenceSelectionTest)
