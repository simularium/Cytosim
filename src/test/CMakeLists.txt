set(TEST_INCLUDES
    "${BASE_SRC_DIR}"
    "${MATH_SRC_DIR}"
    "${SIM_SRC_DIR}"
    "${DISP_SRC_DIR}"
)

set(TEST_LIBS
    "${LAPACK_LIB}"
    "${BLAS_LIB}"
    Threads::Threads
    "${MATH_LIB_TARGET}"
    "${BASE_LIB_TARGET}"
    "${OPENGL_LIBS}"
)

set(TEST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src/test/")

set(TEST_LIST
    #"test" # Test is a reserved word in CMake 3.10.x
    "test_blas"
    "test_glos"
    "test_random"
    "test_quaternion"
    "test_simd"
    "test_sizeof"
    "test_cxx"
    "test_math"
    "test_thread"
    "test_string"
)

foreach(TEST_NAME ${TEST_LIST})
    add_executable("${TEST_NAME}" "${PROJECT_SOURCE_DIR}/src/test/${TEST_NAME}.cc")
    set_source_files_properties(
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    target_include_directories(${TEST_NAME} PUBLIC "${TEST_INCLUDES}")
    target_link_libraries(${TEST_NAME} PUBLIC "${TEST_LIBS}")
endforeach()

set(TEST_GL_LIBS
    rasterizerGL
    "${DISP_LIB_TARGET}"
    "${SPACES_LIB_TARGET}"
    "${SIM_LIB_TARGET}"
    "${MATH_LIB_TARGET}"
    "${BASE_LIB_TARGET}"
    "${LAPACK_LIB}"
    "${BLAS_LIB}"
    Threads::Threads
    "${OPENGL_LIBS}"
)

set(GL_TEST_LIST
    #"test_opengl" #Currently experiencing an issue with glext.h on Linux
    "test_glut"
    "test_glapp"
    "test_sphere"
    "test_rasterizer"
    "test_space"
    "test_grid"
    "test_solve"
)

foreach(TEST_NAME ${GL_TEST_LIST})
    add_executable(${TEST_NAME} "${PROJECT_SOURCE_DIR}/src/test/${TEST_NAME}.cc")
    target_include_directories(${TEST_NAME} PUBLIC "${TEST_INCLUDES}")
    target_link_libraries(${TEST_NAME} PUBLIC "${TEST_GL_LIBS}")
endforeach()
