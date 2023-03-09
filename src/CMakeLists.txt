
add_subdirectory("base")
add_subdirectory("math")
add_subdirectory("sim")

if (MAKE_PLAY)
    add_subdirectory("disp")
    add_subdirectory("play")
endif()

if (MAKE_TOOLS)
    add_subdirectory("tools")
endif()

if (MAKE_TESTS)
    add_subdirectory("test")
endif()
