
all: 
	make -C src all

clean:
	make -C src clean
	#rm -f *.o *.exe *~ ./build/*.o ./build/qe_g$(EXE)

distclean: clean
	rm -f config.h config.mak

test:
	make -C tests test
