# from configure
#CONFIG_NETWORK=yes
#CONFIG_WIN32=yes
#CONFIG_TINY=yes
#CONFIG_DLL=yes

# currently fixes
# Define CONFIG_ALL_MODES to include all edit modes
CONFIG_ALL_MODES=y

# Define CONFIG_UNICODE_JOIN to include unicode bidi/script handling
CONFIG_UNICODE_JOIN=y

# Define CONFIG_ALL_KMAPS it to include generic key map handling
CONFIG_ALL_KMAPS=y

all: 
	make -C src all

clean:
	make -C src clean
	#rm -f *.o *.exe *~ ./build/*.o ./build/qe_g$(EXE)

distclean: clean
	rm -f config.h config.mak

test:
	make -C tests test
