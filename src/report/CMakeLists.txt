add_library(report SHARED
    work_log.cpp
)

target_include_directories(report PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${libharu_SOURCE_DIR}/include
    ${libharu_BINARY_DIR}/include
)

target_link_libraries(report
    storage
    hpdf
)
