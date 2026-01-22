file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/storage/*.hpp")
file(GLOB SOURCE_LIST "*.cpp")

add_library(storage ${SOURCE_LIST} ${HEADER_LIST})

target_include_directories(storage PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(storage PUBLIC nlohmann_json::nlohmann_json)

target_compile_features(storage PUBLIC cxx_std_17)
