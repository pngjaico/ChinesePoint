add_executable(DictionaryRegistryTest
  DictionaryRegistryTest.cpp
  ${REPO_ROOT}/src/util/DictionaryRegistry.cpp
)

target_include_directories(DictionaryRegistryTest PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/stubs
  ${REPO_ROOT}/src
)

target_link_libraries(DictionaryRegistryTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(DictionaryRegistryTest)
