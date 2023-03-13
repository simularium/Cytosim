
set(TOOL_INCLUDES math base sim play)
list(TRANSFORM TOOL_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")

set(TOOL_LIBS
    "${SIM_LIBRARY}"
    "${MATH_LIBRARY}"
    "${BASE_LIBRARY}"
    "${LAPACK_BLAS}"
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
foreach(TOOL ${TOOL_LIST})
    add_executable("${TOOL}" "${PROJECT_SOURCE_DIR}/src/tools/${TOOL}.cc")
    set_source_files_properties(
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    target_link_libraries(${TOOL} PUBLIC "${TOOL_LIBS}")
    target_include_directories(${TOOL} PUBLIC ${TOOL_INCLUDES})
endforeach()

