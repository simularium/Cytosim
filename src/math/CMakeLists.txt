set(MATH_SOURCES
    "${PROJECT_SOURCE_DIR}/src/math/vector1.cc"
    "${PROJECT_SOURCE_DIR}/src/math/vector2.cc"
    "${PROJECT_SOURCE_DIR}/src/math/vector3.cc"
    "${PROJECT_SOURCE_DIR}/src/math/matrix11.cc"
    "${PROJECT_SOURCE_DIR}/src/math/matrix22.cc"
    "${PROJECT_SOURCE_DIR}/src/math/matrix33.cc"
    "${PROJECT_SOURCE_DIR}/src/math/rasterizer.cc"
    "${PROJECT_SOURCE_DIR}/src/math/matsparsesym1.cc"
    "${PROJECT_SOURCE_DIR}/src/math/matsparsesymblk.cc"
    "${PROJECT_SOURCE_DIR}/src/math/polygon.cc"
    "${PROJECT_SOURCE_DIR}/src/math/pointsonsphere.cc"
    "${PROJECT_SOURCE_DIR}/src/math/random.cc"
    "${PROJECT_SOURCE_DIR}/src/math/platonic.cc"
    "${PROJECT_SOURCE_DIR}/src/math/random_vector.cc"
    "${PROJECT_SOURCE_DIR}/src/math/project_ellipse.cc"
    "${PROJECT_SOURCE_DIR}/src/math/SFMT.c"
)

add_library(${MATH_LIB_TARGET} STATIC ${MATH_SOURCES})
target_include_directories(${MATH_LIB_TARGET} PUBLIC
    "${MATH_SRC_DIR}"
    "${BASE_SRC_DIR}"
)

if(BUILD_CYTOSIM_TESTS)
    add_library(rasterizerGL "${PROJECT_SOURCE_DIR}/src/math/rasterizer.cc")
    target_compile_definitions(rasterizerGL PRIVATE -DDISPLAY)
    target_include_directories(rasterizerGL PUBLIC
        "${MATH_SRC_DIR}"
        "${BASE_SRC_DIR}"
        "${DISP_SRC_DIR}"
    )
endif()
