add_executable(tests tests.cpp lzip_t.cpp performance_tests.cpp file_tests.cpp)
target_link_libraries(tests PUBLIC lzip Catch2::Catch2)

# allow user to run tests with `make test` or `ctest`
catch_discover_tests(tests)
