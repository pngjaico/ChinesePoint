add_executable(RecoveryBackupContractTest RecoveryBackupContractTest.cpp)

target_include_directories(RecoveryBackupContractTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(RecoveryBackupContractTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(RecoveryBackupContractTest)
