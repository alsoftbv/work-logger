file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/command/*.hpp")
file(GLOB SOURCE_LIST "*.cpp")

add_library(command ${SOURCE_LIST} ${HEADER_LIST})

target_include_directories(command PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(command PUBLIC CLI11::CLI11 flow invoice)

target_compile_features(command PUBLIC cxx_std_17)
