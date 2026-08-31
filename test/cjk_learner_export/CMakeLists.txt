add_executable(CjkLearnerExportTest
  CjkLearnerExportTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkLearnerExport.cpp
)

target_include_directories(CjkLearnerExportTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkLearnerExportTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkLearnerExportTest)
