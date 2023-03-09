set(SOURCES_DISP
    view.cc
    view_prop.cc
    glapp.cc
    fiber_disp.cc
    line_disp.cc
    point_disp.cc
    grid_display.cc
    display_prop.cc
    display.cc
    display1.cc
    display2.cc
    display3.cc
    offscreen.cc
    save_image.cc
    gle.cc
    gle_color.cc
    gle_color_list.cc
)

list(TRANSFORM SOURCES_DISP PREPEND "${PROJECT_SOURCE_DIR}/src/disp/")

set(SOURCES_GYM_DEPS
    spng.c
    miniz.c
)

list(TRANSFORM SOURCES_GYM_DEPS PREPEND "${PROJECT_SOURCE_DIR}/src/disp/")

set_property(SOURCE spng.c PROPERTY COMPILE_DEFINITIONS SPNG_STATIC SPNG_USE_MINIZ)


set(DISP_INCLUDES math base sim sim/organizers disp)
list(TRANSFORM DISP_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")

add_library(${DISP_LIBRARY} STATIC ${SOURCES_DISP} ${SOURCES_GYM_DEPS})

target_compile_definitions(${DISP_LIBRARY} PRIVATE -DDISPLAY)
target_include_directories(${DISP_LIBRARY} PUBLIC "${DISP_INCLUDES}" )
