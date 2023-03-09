
set(SOURCES_SIM
    modulo.cc movable.cc
    mecable.cc meca.cc mecapoint.cc
    interpolation.cc interpolation4.cc
    fiber_segment.cc fiber_site.cc
    lattice.cc
    object.cc object_set.cc
    sphere_prop.cc sphere.cc sphere_set.cc
    bead.cc bead_set.cc
    solid_prop.cc solid.cc solid_set.cc
    field.cc field_prop.cc field_set.cc
    event.cc event_set.cc
    chain.cc mecafil.cc
    fiber.cc fiber_prop.cc fiber_set.cc
    hand.cc hand_prop.cc hand_monitor.cc
    single.cc single_prop.cc single_set.cc
    couple.cc couple_prop.cc couple_set.cc
    organizer.cc organizer_set.cc
    fiber_grid.cc point_grid.cc
    space.cc space_prop.cc space_set.cc
    simul.cc simul_prop.cc
    interface.cc parser.cc
)

set(SOURCES_SPACES
    space_square.cc
    space_sphere.cc
    space_dice.cc
    space_torus.cc
    space_polygon.cc
    space_banana.cc
    space_ellipse.cc
    space_cylinder.cc
    space_cylinderZ.cc
    space_capsule.cc
    space_strip.cc
    space_periodic.cc
    space_cylinderP.cc
    space_polygonZ.cc
    space_ring.cc
)
list(TRANSFORM SOURCES_SPACES PREPEND "spaces/")

set(SOURCES_HANDS
    motor.cc motor_prop.cc
    slider.cc slider_prop.cc
    actor.cc actor_prop.cc
    nucleator.cc nucleator_prop.cc
    tracker.cc tracker_prop.cc
    cutter.cc cutter_prop.cc
    rescuer.cc rescuer_prop.cc
    mighty.cc mighty_prop.cc
    chewer_prop.cc chewer.cc
    walker_prop.cc walker.cc
    regulator_prop.cc regulator.cc
    digit_prop.cc digit.cc
    myosin_prop.cc myosin.cc
    dynein_prop.cc dynein.cc
    kinesin_prop.cc kinesin.cc
)
list(TRANSFORM SOURCES_HANDS PREPEND "hands/")

set(SOURCES_FIBERS
    dynamic_fiber.cc dynamic_fiber_prop.cc
    classic_fiber.cc classic_fiber_prop.cc
    treadmilling_fiber.cc treadmilling_fiber_prop.cc
    growing_fiber.cc growing_fiber_prop.cc
)
list(TRANSFORM SOURCES_FIBERS PREPEND "fibers/")

set(SOURCES_SINGLES
    picket.cc picket_long.cc
    wrist.cc wrist_long.cc
)
list(TRANSFORM SOURCES_SINGLES PREPEND "singles/")

set(SOURCES_COUPLES
    couple_long.cc
    crosslink.cc crosslink_long.cc crosslink_prop.cc
    bridge.cc bridge_prop.cc
    shackle.cc shackle_long.cc shackle_prop.cc
    fork.cc fork_prop.cc
    duo.cc duo_prop.cc duo_long.cc
)
list(TRANSFORM SOURCES_COUPLES PREPEND "couples/")

set(SOURCES_ORGANIZERS
    aster.cc aster_prop.cc
    nucleus.cc nucleus_prop.cc
    fake.cc fake_prop.cc
    bundle.cc bundle_prop.cc
)
list(TRANSFORM SOURCES_ORGANIZERS PREPEND "organizers/")

# objects containing graphical code under the DISPLAY keyword:
set(SOURCES_WITH_DISPLAY
    ${SOURCES_SPACES}
    space.cc
    field.cc
    fiber_grid.cc
    point_grid.cc
)

set(SOURCES_READ
    frame_reader.cc
    sim_thread.cc
)

set(SOURCES_CYTOSIM
    "${SOURCES_SIM}"
    "${SOURCES_SPACES}"
    "${SOURCES_HANDS}"
    "${SOURCES_FIBERS}"
    "${SOURCES_SINGLES}"
    "${SOURCES_COUPLES}"
    "${SOURCES_ORGANIZERS}"
    "${SOURCES_READ}"
)

set(ALL_SIM_SRC_DIR sim sim/fibers sim/hands sim/singles sim/couples sim/organizers sim/spaces)


list(TRANSFORM SOURCES_CYTOSIM PREPEND "${PROJECT_SOURCE_DIR}/src/sim/")
list(TRANSFORM SOURCES_WITH_DISPLAY PREPEND "${PROJECT_SOURCE_DIR}/src/sim/")


if(MAKE_SIM)

    set(SIM_INCLUDES math base sim "${ALL_SIM_SRC_DIR}")
    list(TRANSFORM SIM_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")

    add_library(${SIM_LIBRARY} STATIC ${SOURCES_CYTOSIM})
    target_include_directories(${SIM_LIBRARY} PUBLIC "${SIM_INCLUDES}" )

    add_executable(${SIM_TARGET} "${PROJECT_SOURCE_DIR}/src/sim/sim.cc")
    include_directories(PUBLIC "${SIM_INCLUDES}" )
    target_link_libraries(${SIM_TARGET} PRIVATE
        "${SIM_LIBRARY}"
        "${MATH_LIBRARY}"
        "${BASE_LIBRARY}"
        "${LAPACK_BLAS}"
        Threads::Threads
    )
    
endif(MAKE_SIM)



if(MAKE_PLAY)

    set(SIMG_INCLUDES math base sim sim/organizers disp)
    list(TRANSFORM SIMG_INCLUDES PREPEND "${PROJECT_SOURCE_DIR}/src/")

    #set(SOURCES_CYTOSIM_NO_DISPLAY ${SOURCES_CYTOSIM})
    #list(REMOVE_ITEM SOURCES_CYTOSIM_NO_DISPLAY ${SOURCES_WITH_DISPLAY})

    add_library(${SIMG_LIBRARY} STATIC ${SOURCES_CYTOSIM})
    target_include_directories(${SIMG_LIBRARY} PUBLIC "${SIMG_INCLUDES}")

    # It is only necessary to compile files in ${SOURCES_WITH_DISPLAY} with this flag
    # but I do not know how to specify this with CMake
    # Hence by setting a target_compile_definitions, all source will be compiled with -DDISPLAY

    target_compile_definitions(${SIMG_LIBRARY} PRIVATE DISPLAY)

    # This below does not work, as it sets the compile options for all targets including the files

    foreach(FILE ${SOURCES_WITH_DISPLAY})
        #set_source_files_properties(${FILE} TARGET_DIRECTORY ${SIMG_LIBRARY} PROPERTIES COMPILE_DEFINITIONS "DISPLAY")
        #message(${FILE} "will be compiled with -DDISPLAY")
    endforeach()

endif(MAKE_PLAY)
