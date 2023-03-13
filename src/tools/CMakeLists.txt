set(SOURCES_TOOL
    "${PROJECT_SOURCE_DIR}/src/play/frame_reader.cc"
)

set(TOOL_INCLUDES
    "${MATH_SRC_DIR}"
    "${BASE_SRC_DIR}"
    "${SIM_SRC_DIR}"
    "${DISP_SRC_DIR}"
    "${PLAY_SRC_DIR}"
)

set(TOOL_LIBS
    "${SIM_LIB_TARGET}"
    "${SPACES_LIB_TARGET}"
    "${MATH_LIB_TARGET}"
    "${BASE_LIB_TARGET}"
    "${LAPACK_LIB}"
    "${BLAS_LIB}"
    Threads::Threads
)

set(TOOL_LIST
    "frametool"
    "sieve"
    "report"
    "reportF"
	"reader"
)

# Build the Tools
foreach(TOOL_NAME ${TOOL_LIST})
    add_executable("${TOOL_NAME}" ${SOURCES_TOOL} "${PROJECT_SOURCE_DIR}/src/tools/${TOOL_NAME}.cc")
    set_source_files_properties(
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    target_link_libraries(${TOOL_NAME} PUBLIC "${TOOL_LIBS}")
    target_include_directories(${TOOL_NAME} PUBLIC ${TOOL_INCLUDES})
endforeach()

