
set(TEST_INCLUDES math base sim disp)
list(TRANSFORM TEST_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")


set(TEST_LIBS
    "${LAPACK_BLAS}"
    Threads::Threads
    "${MATH_LIBRARY}"
    "${BASE_LIBRARY}"
)

set(TEST_LIST
    #"test" # Test is a reserved word in CMake 3.10.x
    "test_blas"
    "test_glossary"
    "test_random"
    "test_quaternion"
    "test_simd"
    "test_sizeof"
    "test_cxx"
    "test_math"
    "test_thread"
    "test_string"
)

foreach(TEST ${TEST_LIST})
    add_executable("${TEST}" "${PROJECT_SOURCE_DIR}/src/test/${TEST}.cc")
    set_source_files_properties(
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    target_include_directories(${TEST} PUBLIC "${TEST_INCLUDES}")
    target_link_libraries(${TEST} PUBLIC "${TEST_LIBS}")
endforeach()

if(OPENGL_LIBS)

set(TEST_GL_LIBS
    rasterizerGL
    "${GYM_LIBRARY}"
    "${SIMG_LIBRARY}"
    "${MATH_LIBRARY}"
    "${BASE_LIBRARY}"
    "${LAPACK_BLAS}"
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

foreach(TEST ${GL_TEST_LIST})
    add_executable(${TEST} "${PROJECT_SOURCE_DIR}/src/test/${TEST}.cc")
    target_include_directories(${TEST} PUBLIC "${TEST_INCLUDES}")
    target_link_libraries(${TEST} PUBLIC "${TEST_GL_LIBS}")
endforeach()

endif()
