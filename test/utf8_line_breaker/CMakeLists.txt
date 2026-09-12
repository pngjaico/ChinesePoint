add_executable(Utf8LineBreakerTest Utf8LineBreakerTest.cpp)

target_include_directories(Utf8LineBreakerTest PRIVATE
  ${REPO_ROOT}/src
)

target_link_libraries(Utf8LineBreakerTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(Utf8LineBreakerTest)
