add_executable(CjkLookupCandidatesTest
  CjkLookupCandidatesTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkLookupCandidates.cpp
)

target_include_directories(CjkLookupCandidatesTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkLookupCandidatesTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkLookupCandidatesTest)
