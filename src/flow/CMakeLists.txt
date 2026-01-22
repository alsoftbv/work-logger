file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/flow/*.hpp")
file(GLOB SOURCE_LIST "*.cpp")

add_library(flow ${SOURCE_LIST} ${HEADER_LIST})

target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(flow PUBLIC storage)

target_compile_features(flow PUBLIC cxx_std_17)
