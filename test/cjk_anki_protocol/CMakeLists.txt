add_executable(cjk_anki_protocol_tests CjkAnkiProtocolTest.cpp
  ../../src/chinesepoint/cjk/CjkAnkiProtocol.cpp)
target_include_directories(cjk_anki_protocol_tests PRIVATE ../../src)
target_link_libraries(cjk_anki_protocol_tests GTest::gtest_main)
gtest_discover_tests(cjk_anki_protocol_tests)
