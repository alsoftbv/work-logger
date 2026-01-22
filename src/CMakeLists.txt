add_subdirectory(storage)
add_subdirectory(flow)
add_subdirectory(invoice)
add_subdirectory(report)
add_subdirectory(command)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
