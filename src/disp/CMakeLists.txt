set(SOURCES_DISP
    "${PROJECT_SOURCE_DIR}/src/disp/gle.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/gle_color.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/gle_color_list.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/view.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/view_prop.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/grid_display.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/glapp.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/saveimage.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/offscreen.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/fiber_disp.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/line_disp.cc"
    "${PROJECT_SOURCE_DIR}/src/disp/point_disp.cc"
)

add_library(${DISP_LIB_TARGET} STATIC ${SOURCES_DISP})

target_include_directories(${DISP_LIB_TARGET} PUBLIC
    "${MATH_SRC_DIR}"
    "${BASE_SRC_DIR}"
    "${SIM_SRC_DIR}"
)