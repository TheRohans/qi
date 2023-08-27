.PHONY: test

include config.mak

# from configure / config.mak
#CONFIG_TINY=yes

# currently fixes
# Define CONFIG_ALL_MODES to include all edit modes
CONFIG_ALL_MODES=y

# Define CONFIG_UNICODE_JOIN to include unicode bidi/script handling
CONFIG_UNICODE_JOIN=n

# Define CONFIG_ALL_KMAPS it includes generic key map handling
CONFIG_ALL_KMAPS=n

CFLAGS:=-Wall
# Note: Using c99 (or any ansi version) with signals is a mine field of terrible.
# CFLAGS:=-std=c99 -Wall -Wpedantic -Werror -pedantic -pedantic-errors
# https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error
# CFLAGS:=-std=c99 -Wall -Wpedantic -pedantic -D_XOPEN_SOURCE=700

ifdef CONFIG_DEBUG
CFLAGS+= -O0 -g 
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

LIBS+=-lm

LOAD_PATH=../build

#############################################################################
#############################################################################
#############################################################################

TARGETLIBS:=
TARGETS+=qi$(EXE)

# qfribidi.o == CONFIG_UNICODE_JOIN qstack.o qfribidi.o input.o
_OBJS=qe.o buffer.o unicode.o display.o \
util.o cutils.o unix.o

OBJS = $(patsubst %,$(LOAD_PATH)/%,$(_OBJS))

TEST_SRC=$(wildcard ./tests/*.c)

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
	$(MAKE) -C plugins core
ifndef CONFIG_TINY
	$(MAKE) -C plugins extra
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
	$(MAKE) -C plugins clean
	rm -f *.o *.exe *~ ../build/*.o ../build/qe_g$(EXE) test
	rm -f ../qi$(EXE)

distclean: clean
	rm -f config.h config.mak

install:
	cp ../dist/qi $(PREFIX)/qi

test:
	@echo '======================================== Building qi tests ==== '
	mkdir -p ../build
	$(CC) $(CFLAGS) $(TESTS_SRC) tests.c $(LIBS) -o ../build/run_tests
	$(MAKE) -C plugins test
