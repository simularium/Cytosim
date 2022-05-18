# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.

#
# File src/disp/makefile.inc

OBJ_DISP := fiber_disp.o line_disp.o point_disp.o grid_display.o\
            gle.o gle_color.o gle_color_list.o view.o view_prop.o glapp.o


#-----------------------LIB & DEF for PNG support-------------------------------

IMAGES=
IMAGES_DEF=

ifneq ($(HAS_PNG), 0)

    IMAGES_DEF+=-DHAS_PNG $(INC_PNG)
    IMAGES+=$(LIB_PNG)

endif


#----------------------------targets--------------------------------------------

cytodisp.a: $(OBJ_DISP) save_image.o offscreen.o libspng.o miniz.o
	$(MAKELIB)
	$(DONE)


# Attention: FreeGLUT font are coded in C-language (use -x c):

fg_font.o: fg_font.c fg_font.h
	$(CXX) -DNDEBUG $(FAST) -x c -c $< -o build/$@

fg_stroke_roman.o: fg_stroke_roman.c fg_font.h
	$(CXX) -DNDEBUG $(FAST) -x c -c $< -o build/$@

fg_stroke_mono_roman.o: fg_stroke_mono_roman.c fg_font.h
	$(CXX) -DNDEBUG $(FAST) -x c -c $< -o build/$@


miniz.o: miniz.c miniz.h
	$(CXX) $(FAST) -x c -c $< -o build/$@

libspng.o: spng.c spng.h
	$(CXX) -DSPNG_USE_MINIZ -DSPNG_STATIC -DSPNG_USE_MINIZ $(FAST) -x c -c $< -o build/$@

save_image.o: save_image.cc save_image.h
	$(COMPILE) -DDISPLAY $(IMAGES_DEF) -c $< -o build/$@


offscreen.o: offscreen.cc offscreen.h offscreen_fbo.cc offscreen_glx.cc
	$(COMPILE) -c $< -o build/$@


$(OBJ_DISP): %.o: %.cc %.h | build
	$(COMPILE) -Isrc/math -Isrc/base -Isrc/sim -c $< -o build/$@
