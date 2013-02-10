include config.mak

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

CFLAGS:=-Wall -g $(CFLAGS) -funsigned-char -I$(LOAD_PATH):./ #-arch i386 -m32
#LDFLAGS+= -arch i386 -m32
ifdef TARGET_GPROF
CFLAGS+= -p
LDFLAGS+= -p
endif
ifdef TARGET_ARCH_X86
#CFLAGS+=-fomit-frame-pointer
#ifeq ($(GCC_MAJOR),2)
#CFLAGS+=-m386 -malign-functions=0
#else
CFLAGS+=-march=i386 -falign-functions=0
#endif
endif
DEFINES=-DHAVE_QE_CONFIG_H

LOAD_PATH=../build

#############################################################################
#############################################################################
#############################################################################

ifdef CONFIG_DLL
LIBS+=-ldl
# export some qemacs symbols
# -E parameter doesn't work on OS X
#LDFLAGS+=-Wl,-E
LDFLAGS+=-Wl
endif

LIBS+=-lm

TARGETLIBS:=
TARGETS+=qi$(EXE)

_OBJS=qe.o charset.o buffer.o input.o unicode_join.o display.o util.o cutils.o

ifndef CONFIG_WIN32
_OBJS+= unix.o
endif

_OBJS+= qfribidi.o

ifdef CONFIG_WIN32
_OBJS+= win32.o
LIBS+= -lgdi32
endif

#ifdef CONFIG_UNICODE_JOIN
#_OBJS+= arabic.o indic.o qfribidi.o plugin_unihex.o
#endif

OBJS = $(patsubst %,$(LOAD_PATH)/%,$(_OBJS))

# Inlucde all the plugin libraries (extras if not doing a tiny build)
PLUGS=-lplugincore
ifndef CONFIG_TINY
PLUGS+= -lplugins
endif

#############################################################################
#############################################################################
#############################################################################

all: build_plugins $(TARGETLIBS) $(TARGETS)

build_plugins:
	
	make -C plugins core
ifndef CONFIG_TINY
	make -C plugins extra
endif

qe_g$(EXE): $(OBJS) 
	@echo '======================================== Building qi == '
	$(CC) $(LDFLAGS) -L$(LOAD_PATH) $(PLUGS) -o $(LOAD_PATH)/$@ $^ $(LIBS)

qi$(EXE): qe_g$(EXE)
	rm -f ../$@
	cp $(LOAD_PATH)/$< ../$@
ifdef CONFIG_TINY
	strip ../$@
endif
	@echo ''
	@echo '** Your qi is strong. **'
	@ls -alFh ../$@
	@echo ''

$(LOAD_PATH)/%.o : %.c
	@echo '======================================== Building qi part $@ == '
	$(CC) $(DEFINES) $(CFLAGS) -o $@ -c $<
	@echo ''

clean:
	make -C plugins clean
	@echo '======================================== Cleaning qi == '
	rm -f *.o *.exe *~ ../build/*.o ../build/qe_g$(EXE)
	rm -f ../qi$(EXE)

distclean: clean
	rm -f config.h config.mak

test:
	make -C tests test

