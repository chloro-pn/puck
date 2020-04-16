set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

aux_source_directory(. SRC_LIST)

add_library(pnlog ${SRC_LIST})

target_link_libraries(pnlog pthread)
