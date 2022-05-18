# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.
# Copyright 2018-- Francois J. Nedelec.


OBJ_SPACE := space.o space_prop.o space_square.o space_sphere.o\
             space_dice.o space_torus.o space_polygon.o\
             space_polygonZ.o space_ellipse.o space_cylinder.o space_ring.o\
             space_cylinderZ.o space_capsule.o space_strip.o space_periodic.o\
             space_banana.o space_cylinderP.o

OBJ_HANDS := hand.o hand_prop.o\
             motor.o motor_prop.o\
             slider.o slider_prop.o\
             actor.o actor_prop.o\
             nucleator.o nucleator_prop.o\
             tracker.o tracker_prop.o\
             cutter.o cutter_prop.o\
             chewer.o chewer_prop.o\
             rescuer.o rescuer_prop.o\
             regulator.o regulator_prop.o\
             mighty.o mighty_prop.o

OBJ_DIGITS := digit.o digit_prop.o\
              walker.o walker_prop.o\
              myosin.o myosin_prop.o\
              dynein.o dynein_prop.o\
              kinesin.o kinesin_prop.o

OBJ_SINGLES := single.o single_prop.o single_set.o\
               picket.o picket_long.o wrist.o wrist_long.o

OBJ_COUPLES := couple.o couple_prop.o couple_set.o couple_long.o\
               crosslink.o crosslink_long.o crosslink_prop.o\
               bridge.o bridge_prop.o\
               duo.o duo_long.o duo_prop.o\
               shackle.o shackle_long.o shackle_prop.o\
               fork.o fork_prop.o
         
OBJ_FIBERS := chain.o fiber.o mecafil.o fiber_prop.o fiber_set.o\
              fiber_segment.o fiber_site.o lattice.o\
              dynamic_fiber.o dynamic_fiber_prop.o\
              classic_fiber.o classic_fiber_prop.o\
              treadmilling_fiber.o treadmilling_fiber_prop.o\
              growing_fiber.o growing_fiber_prop.o

OBJ_ORGANIZERS := organizer.o organizer_set.o \
                  aster.o aster_prop.o nucleus.o nucleus_prop.o\
                  fake.o fake_prop.o bundle.o bundle_prop.o

OBJ_SIM := modulo.o movable.o mecable.o object.o object_set.o\
           sphere_prop.o sphere.o sphere_set.o\
           solid_prop.o solid.o solid_set.o bead.o bead_set.o\
           field.o field_prop.o field_set.o\
           event.o event_set.o\
           mecapoint.o interpolation.o interpolation4.o\
           meca.o fiber_grid.o point_grid.o space_set.o\
           simul_prop.o simul.o interface.o parser.o


OBJ_CYTOSIM:=$(OBJ_SPACE) $(OBJ_SIM) $(OBJ_HANDS) $(OBJ_DIGITS) $(OBJ_FIBERS)\
             $(OBJ_SINGLES) $(OBJ_COUPLES) $(OBJ_ORGANIZERS)

OBJ_CYTOSIMD1:=$(OBJ_CYTOSIM:%.o=%D1.o)
OBJ_CYTOSIMD2:=$(OBJ_CYTOSIM:%.o=%D2.o)
OBJ_CYTOSIMD3:=$(OBJ_CYTOSIM:%.o=%D3.o)


# objects containing graphical code under the DISPLAY keyword:
OBJ_GRAFIX:=$(OBJ_SPACE) field.o fiber_grid.o point_grid.o
OBJ_CYTOSIMG:=$(filter-out $(OBJ_GRAFIX),$(OBJ_CYTOSIM)) $(OBJ_GRAFIX:%.o=%G.o)

#----------------------------targets--------------------------------------------

INC_BM:=-Isrc/base -Isrc/math
INC_BMD=$(addprefix -Isrc/, base math sim sim/spaces disp)
INC_SIM=$(addprefix -Isrc/, base math sim sim/fibers sim/hands sim/singles sim/couples sim/organizers sim/spaces)

#--- normal build:

sim: sim.cc cytosim.a cytomath.a cytobase.a SFMT.o | bin
	$(COMPILE) $(INC_BM) $(INFO) $(OBJECTS) $(LINK) -o bin/$@
	$(DONE)
vpath sim  bin

#--- build for debuging:

simd: sim.cc cytosim.a cytomath.a cytobase.a SFMT.o | bin
	$(CXX) $(FlagsD) $(INC_BM) $(INFO) $(OBJECTS) $(LINK) -o bin/$@
	$(DONE)
vpath simd  bin

#--- builds with specific dimension:

bin1/sim: sim.cc cytosimD1.a cytomathD1.a cytobase.a SFMT.o | bin1
	$(COMPILE) $(INC_BM) $(INFO) -DDIM=1 $(OBJECTS) $(LINK) -o bin1/sim
	$(DONE)

bin2/sim: sim.cc cytosimD2.a cytomathD2.a cytobase.a SFMT.o | bin2
	$(COMPILE) $(INC_BM) $(INFO) -DDIM=2 $(OBJECTS) $(LINK) -o bin2/sim
	$(DONE)

bin3/sim: sim.cc cytosimD3.a cytomathD3.a cytobase.a SFMT.o | bin3
	$(COMPILE) $(INC_BM) $(INFO) -DDIM=3 $(OBJECTS) $(LINK) -o bin3/sim
	$(DONE)


#--- libraries:

cytosim.a: $(OBJ_CYTOSIM) | lib
	$(MAKELIB)
	$(DONE)

cytosimD1.a: $(OBJ_CYTOSIMD1) | lib
	$(MAKELIB)
	$(DONE)

cytosimD2.a: $(OBJ_CYTOSIMD2) | lib
	$(MAKELIB)
	$(DONE)

cytosimD3.a: $(OBJ_CYTOSIMD3) | lib
	$(MAKELIB)
	$(DONE)

#--- libraries with graphical code enabled

cytosimG.a: $(OBJ_CYTOSIMG) | lib
	$(MAKELIB)
	$(DONE)

cytosimGD1.a: $(OBJ_CYTOSIMG:%.o=%D1.o) | lib
	$(MAKELIB)
	$(DONE)

cytosimGD2.a: $(OBJ_CYTOSIMG:%.o=%D2.o) | lib
	$(MAKELIB)
	$(DONE)

cytosimGD3.a: $(OBJ_CYTOSIMG:%.o=%D3.o) | lib
	$(MAKELIB)
	$(DONE)

#----------------------------target rules---------------------------------------

$(OBJ_CYTOSIM): %.o: %.cc %.h | build
	$(COMPILE) $(INC_SIM) -c $< -o build/$@

$(OBJ_CYTOSIMD1): %D1.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=1 $(INC_SIM) -c $< -o build/$@

$(OBJ_CYTOSIMD2): %D2.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=2 $(INC_SIM) -c $< -o build/$@

$(OBJ_CYTOSIMD3): %D3.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=3 $(INC_SIM) -c $< -o build/$@


# builds with DISPLAY enabled:

$(OBJ_GRAFIX:%.o=%G.o): %G.o: %.cc %.h %.o | build
	$(COMPILE) -DDISPLAY $(INC_BMD) -c $< -o build/$@

$(OBJ_GRAFIX:%.o=%GD1.o): %GD1.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=1 -DDISPLAY $(INC_BMD) -c $< -o build/$@

$(OBJ_GRAFIX:%.o=%GD2.o): %GD2.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=2 -DDISPLAY $(INC_BMD) -c $< -o build/$@

$(OBJ_GRAFIX:%.o=%GD3.o): %GD3.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=3 -DDISPLAY $(INC_BMD) -c $< -o build/$@

