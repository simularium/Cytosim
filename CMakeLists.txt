cmake_minimum_required(VERSION 3.2.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_BUILD_TYPE Fast)

set(TARGET "cytosim")
project(${TARGET} C CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tar)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if(NOT DEFINED DIMENSION)
    set(DIMENSION 2)
endif()

add_definitions(-DDIM=${DIMENSION})

option(BUILD_SIM_EXECUTABLE "build the cytosim simulation executable" ON)
option(BUILD_PLAY_EXECUTABLE "build the cytosim OpenGL viewer" ON)
option(BUILD_CYTOSIM_TESTS "build the cytosim test executables" ON)
option(BUILD_CYTOSIM_TOOLS "build the cytosim tools executables" ON)

if(BUILD_CYTOSIM_TOOLS)
    set(BUILD_PLAY_EXECUTABLE ON)
endif()

set(BASE_SRC_DIR "${PROJECT_SOURCE_DIR}/src/base")
set(DOC_SRC_DIR "${PROJECT_SOURCE_DIR}/src/doc")
set(DISP_SRC_DIR "${PROJECT_SOURCE_DIR}/src/disp")
set(MATH_SRC_DIR "${PROJECT_SOURCE_DIR}/src/math")
set(PLAY_SRC_DIR "${PROJECT_SOURCE_DIR}/src/play")

set(SIM_SRC_DIR
"${PROJECT_SOURCE_DIR}/src/sim"
"${PROJECT_SOURCE_DIR}/src/sim/fibers"
"${PROJECT_SOURCE_DIR}/src/sim/hands"
"${PROJECT_SOURCE_DIR}/src/sim/singles"
"${PROJECT_SOURCE_DIR}/src/sim/couples"
"${PROJECT_SOURCE_DIR}/src/sim/organizers"
"${PROJECT_SOURCE_DIR}/src/sim/spaces"
)
set(TEST_SRC_DIR "${PROJECT_SOURCE_DIR}/src/test")
set(TOOLS_SRC_DIR "${PROJECT_SOURCE_DIR}/src/tools")

set(CYTOSIM_EXE_TARGET "sim")
set(PLAY_EXE_TARGET "play")

set(SIM_LIB_TARGET "cytosimD${DIMENSION}")
set(BASE_LIB_TARGET "cytobase")
set(MATH_LIB_TARGET "cytomath")
set(SPACES_LIB_TARGET "cytospaceD${DIMENSION}")
set(SPACES_G_LIB_TARGET "cytospaceD${DIMENSION}G")
set(DISP_LIB_TARGET "cytodisp")

# Use this to link against the Cytosim Libraries
set(CYTOSIM_LIBRARIES
    "${SIM_LIB_TARGET}"
    "${SPACES_LIB_TARGET}"
    "${MATH_LIB_TARGET}"
    "${BASE_LIB_TARGET}"
    "${LAPACK_LIB}"
    "${BLAS_LIB}"
    Threads::Threads
    PARENT_SCOPE
)

find_library(LAPACK_LIB lapack REQUIRED)
if(LAPACK_LIB)
    message("Found LAPACK: ${LAPACK_LIB}")
endif()

if(NOT LAPACK_LIB)
    message(FATAL_ERROR "LAPACK library not found")
endif()

find_library(BLAS_LIB blas REQUIRED)
if(BLAS_LIB)
    message("Found BLAS: ${BLAS_LIB}")
endif()

if(NOT BLAS_LIB)
    message(FATAL_ERROR "BLAS library not found")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT UNIX)
    message(FATAL_ERROR "CMake File currently only setup to build on Linux and Mac")
endif()

if(BUILD_PLAY_EXECUTABLE OR BUILD_CYTOSIM_TESTS OR BUILD_CYTOSIM_TOOLS)
    if(UNIX AND NOT APPLE)
        message("Building for Linux")
        find_package(GLUT)
        find_package(OpenGL)
        find_package(GLU)
        find_package(Xt)
        find_package(X11)
        find_package(GLEW)

        set(OPENGL_LIBS
            glut
            OpenGL::GL
            GLU
            Xt
            X11
            GLEW
        )
    elseif(UNIX AND APPLE)
        message("Building for Mac")

        set(OPENGL_LIBS
            "-framework Accelerate"
            "-framework GLUT"
            "-framework OpenGL"
            "-framework AGL"
        )
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CXX_FLAGS "-Wno-deprecated-declarations")
    set(FAST_FLAGS "-Wno-deprecated-declarations -ffast-math -funroll-loops")
    set(DEBUG_FLAGS "-g3 -ggdb")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CXX_FLAGS "-Wno-deprecated-declarations")
    set(FAST_FLAGS "-Wno-deprecated-declarations -ffast-math -funroll-loops")
    set(DEBUG_FLAGS "-g3 -ggdb")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CXX_FLAGS "-fprofile-arcs -ftest-coverage")
    set(FAST_FLAGS "-O3 -fno-trapping-math -fno-tree-vectorize -ffast-math -funroll-loops -march=native -Wno-deprecated")
    #set(FAST_FLAGS "-O3 -fno-tree-vectorize  -fno-trapping-math -ffast-math -funroll-loops -march=native  -floop-optimize -ftree-vectorize -mavx2 -mfma -fno-trapping-math")
    set(DEBUG_FLAGS "-g3 -ggdb -Wno-deprecated")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    set(CXX_FLAGS "-no-parallel -mkl=sequential -wd1224,161")
    set(FAST_FLAGS "-O3 -mdynamic-no-pic -funroll-loops -fno-math-errno -mavx")
    set(DEBUG_FLAGS "-g -Wall -no-pie -wd279,383,810,869,981,1418,1419,1572,2259")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message(FATAL_ERROR "Microsoft Visual Studio not supported for compilation")
else()
    message(FATAL_ERROR "Unrecognized compiler used: ${CMAKE_CXX_COMPILER_ID}")
endif()


file(COPY ${PROJECT_SOURCE_DIR}/cym DESTINATION ${PROJECT_BINARY_DIR})
add_definitions("-finline-functions ${FAST_FLAGS}")
add_subdirectory("src")
