add_executable(CjkLearnerRepositoryTest
  CjkLearnerRepositoryTest.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkJournal.cpp
  ${REPO_ROOT}/src/chinesepoint/cjk/CjkLearnerRepository.cpp
)

target_include_directories(CjkLearnerRepositoryTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(CjkLearnerRepositoryTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(CjkLearnerRepositoryTest)
