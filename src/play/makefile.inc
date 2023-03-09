# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

OBJ_PLAY := display_prop.o player_prop.o sim_thread.o frame_reader.o\
            display.o display1.o display2.o display3.o

FILES_PLAYER := player.cc player.h player_disp.cc

OBJ_PLAY1D := $(OBJ_PLAY:.o=D1.o)
OBJ_PLAY2D := $(OBJ_PLAY:.o=D2.o)
OBJ_PLAY3D := $(OBJ_PLAY:.o=D3.o)

PLAY_INC = $(addprefix -Isrc/, math base sim disp sim/organizers sim/singles)
PLAY_DEP = $(addprefix -Isrc/, math base sim disp) $(LINK) $(GRAFIX) $(IMAGES)

################################ objects with DIM defined in source code:

player.o: $(FILES_PLAYER) | build
	$(COMPILE) $(PLAY_INC) -c $< -o build/$@

$(OBJ_PLAY): %.o: %.cc %.h | build
	$(COMPILE) $(PLAY_INC) -c $< -o build/$@

play: play.cc player.o $(OBJ_PLAY) cytosimG.a cytodisp.a cytomath.a cytobase.a SFMT.o | bin
	$(COMPILE) $(INFO) $(OBJECTS) $(PLAY_DEP) -o bin/play
	$(DONE)
vpath play bin

################################ objects with DIM = 1:

playerD1.o: $(FILES_PLAYER) | build
	$(COMPILE) -DDIM=1 $(PLAY_INC) -c $< -o build/$@

$(OBJ_PLAY1D): %D1.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=1 $(PLAY_INC) -c $< -o build/$@

bin1/play: play.cc playerD1.o $(OBJ_PLAY1D) cytosimGD1.a cytodisp.a cytomathD1.a cytobase.a SFMT.o | bin1
	$(COMPILE) -DDIM=1 $(INFO) $(OBJECTS) $(PLAY_DEP) -o bin1/play
	$(DONE)
vpath bin1/play bin1

################################ objects with DIM = 2:

playerD2.o: $(FILES_PLAYER) | build
	$(COMPILE) -DDIM=2 $(PLAY_INC) -c $< -o build/$@

$(OBJ_PLAY2D): %D2.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=2 $(PLAY_INC) -c $< -o build/$@

bin2/play: play.cc playerD2.o $(OBJ_PLAY2D) cytosimGD2.a cytodisp.a cytomathD2.a cytobase.a SFMT.o | bin2
	$(COMPILE) -DDIM=2 $(INFO) $(OBJECTS) $(PLAY_DEP) -o bin2/play
	$(DONE)
vpath bin2/play bin2

################################ objects with DIM = 3:

playerD3.o: $(FILES_PLAYER) | build
	$(COMPILE) -DDIM=3 $(PLAY_INC) -c $< -o build/$@

$(OBJ_PLAY3D): %D3.o: %.cc %.h %.o | build
	$(COMPILE) -DDIM=3 $(PLAY_INC) -c $< -o build/$@

bin3/play: play.cc playerD3.o $(OBJ_PLAY3D) cytosimGD3.a cytodisp.a cytomathD3.a cytobase.a SFMT.o | bin3
	$(COMPILE) -DDIM=3 $(INFO) $(OBJECTS) $(PLAY_DEP) -o bin3/play
	$(DONE)
vpath bin3/play bin3

