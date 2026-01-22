file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/invoice/*.hpp")
file(GLOB SOURCE_LIST "*.cpp")

add_library(invoice ${SOURCE_LIST} ${HEADER_LIST})

target_include_directories(invoice PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${libharu_SOURCE_DIR}/include
    ${libharu_BINARY_DIR}/include
)

target_link_libraries(invoice PUBLIC storage hpdf)

target_compile_features(invoice PUBLIC cxx_std_17)
