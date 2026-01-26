add_executable(wlog wlog.cpp)
target_compile_features(wlog PRIVATE cxx_std_17)

target_link_libraries(wlog PRIVATE CLI11::CLI11 command storage)