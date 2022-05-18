# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_BASE := messages.o filewrapper.o filepath.o iowrapper.o exceptions.o\
            tictoc.o node_list.o inventory.o stream_func.o tokenizer.o\
            glossary.o property.o property_list.o backtrace.o print_color.o\

#----------------------------rules----------------------------------------------

$(OBJ_BASE): %.o: %.cc %.h | build
	$(COMPILE) -Isrc/base -Isrc/math -c $< -o build/$@

operator_new.o: operator_new.cc
	$(COMPILE) -Isrc/base -Isrc/math -c $< -o build/$@

#----------------------------targets--------------------------------------------

cytobase.a: $(OBJ_BASE) operator_new.o | lib
	$(MAKELIB)
	$(DONE)

