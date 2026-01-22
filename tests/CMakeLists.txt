include(GoogleTest)

add_executable(wlog_tests
    test_config.cpp
    test_client.cpp
    test_invoice.cpp
    test_work_log.cpp
)

target_link_libraries(wlog_tests PRIVATE
    GTest::gtest_main
    storage
    invoice
    report
)

target_include_directories(wlog_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

gtest_discover_tests(wlog_tests)
