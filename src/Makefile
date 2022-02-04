include config.mak

# from configure / config.mak
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

CFLAGS:=-Wall
# CFLAGS:=-std=c99 -Wall -Wpedantic -Werror -pedantic -pedantic-errors
# https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error
# CFLAGS:=-std=c99 -Wall -Wpedantic -pedantic -D_XOPEN_SOURCE=700

ifdef CONFIG_DEBUG
CFLAGS+= -Og -g 
else 
CFLAGS+= -Os
endif 

CFLAGS+= $(CFLAGS) -funsigned-char -I$(LOAD_PATH):./

ifdef TARGET_GPROF
CFLAGS+= -p
LDFLAGS+= -p
endif

ifdef TARGET_ARCH_X86
CFLAGS+=-march=i386 -falign-functions=0
endif

DEFINES=-DHAVE_QE_CONFIG_H

ifdef CONFIG_DLL
LIBS+=-ldl
LDFLAGS+=-Wl
endif

#ifdef CONFIG_WIN32
#_OBJS+= win32.o
#LIBS+= -lgdi32
#endif

#ifdef CONFIG_UNICODE_JOIN
#_OBJS+= arabic.o indic.o qfribidi.o plugin_unihex.o
#endif

LIBS+=-lm

LOAD_PATH=../build

#############################################################################
#############################################################################
#############################################################################

TARGETLIBS:=
TARGETS+=qi$(EXE)

_OBJS=qe.o charset.o buffer.o input.o unicode_join.o display.o util.o cutils.o qstack.o

ifndef CONFIG_WIN32
_OBJS+= unix.o
endif

_OBJS+= qfribidi.o

OBJS = $(patsubst %,$(LOAD_PATH)/%,$(_OBJS))

TEST_SRC=$(wildcard *_test.c)

# Inlucde all the plugin libraries (extras if not doing a tiny build)
PLUGS=-lplugincore
ifndef CONFIG_TINY
PLUGS+= -lplugins
endif

export CC
export CFLAGS

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
	$(CC) $(LDFLAGS) -o $(LOAD_PATH)/$@ $^ $(LIBS) -L$(LOAD_PATH) $(PLUGS) 

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
	rm -f *.o *.exe *~ ../build/*.o ../build/qe_g$(EXE) test
	rm -f ../qi$(EXE)

distclean: clean
	rm -f config.h config.mak

#############################################################################

test: $(TEST_SRC)
	make -C plugins test
