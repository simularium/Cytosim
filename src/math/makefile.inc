# Cytosim was created by Francois Nedelec. Copyright 2007-2017 EMBL.


OBJ_MATH := vector1.o vector2.o vector3.o matrix11.o matrix22.o matrix33.o\
    	rasterizer.o project_ellipse.o platonic.o matrix.o matsparse.o matsparsesym.o\
    	matsparsesym1.o polygon.o pointsonsphere.o random.o random_vector.o

#----------------------------rules----------------------------------------------

INC_BM:=-Isrc/base -Isrc/math


$(OBJ_MATH): %.o: %.cc %.h | build
	$(COMPILE) $(INC_BM) -c $< -o build/$@


matsparsesym2.o: %.o: %.cc %.h | build
	$(COMPILE) $(INC_BM) -c $< -o build/$@


matsparsesymblk.o: matsparsesymblk.cc matsparsesymblk.h
	$(COMPILE) $(INC_BM) -c $< -o build/$@

matsparsesymblkD1.o: matsparsesymblk.cc matsparsesymblk.h
	$(COMPILE) $(INC_BM) -DBLOCK_SIZE=1 -c $< -o build/$@

matsparsesymblkD2.o: matsparsesymblk.cc matsparsesymblk.h
	$(COMPILE) $(INC_BM) -DBLOCK_SIZE=2 -c $< -o build/$@

matsparsesymblkD3.o: matsparsesymblk.cc matsparsesymblk.h
	$(COMPILE) $(INC_BM) -DBLOCK_SIZE=3 -c $< -o build/$@


cytomath.a: $(OBJ_MATH) matsparsesymblk.o | lib
	$(MAKELIB)
	$(DONE)

cytomathD1.a: $(OBJ_MATH) matsparsesymblkD1.o
	$(MAKELIB)
	$(DONE)

cytomathD2.a: $(OBJ_MATH) matsparsesymblkD2.o
	$(MAKELIB)
	$(DONE)

cytomathD3.a: $(OBJ_MATH) matsparsesymblkD3.o
	$(MAKELIB)
	$(DONE)

