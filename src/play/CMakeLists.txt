
set(SOURCES_PLAY
    player_prop.cc
    player.cc
    play.cc
)

list(TRANSFORM SOURCES_PLAY PREPEND "${PROJECT_SOURCE_DIR}/src/play/")

if(MAKE_PLAY)
    add_executable(${PLAY_TARGET} ${SOURCES_PLAY})

    target_link_libraries(${PLAY_TARGET} PUBLIC
        "${SIMG_LIBRARY}"
        "${DISP_LIBRARY}"
        "${GYM_LIBRARY}"
        "${MATH_LIBRARY}"
        "${BASE_LIBRARY}"
        "${OPENGL_LIBS}"
        "${LAPACK_BLAS}"
        Threads::Threads
    )
    
    set(PLAY_INCLUDES math base sim gym disp)
    list(TRANSFORM PLAY_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")

    target_include_directories(${PLAY_TARGET} PUBLIC "${PLAY_INCLUDES}")
endif(MAKE_PLAY)
