#
# Qi sample Makefile to develop plugins
#
# Qi plugins can contain modes, bindings, or any other qemacs
# dynamically registerable resource.

# modify to get the path of the qemacs sources
QEMACS_PATH=../../
LOAD_PATH=../../build/plugins/
# define the name of your plugin
LIB=my_plugin.so
# define the object files it contains
OBJS=my_plugin.o

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
DIST_LIB:=$(HOME)/.qi/$(LIB)
CC=gcc
CFLAGS=-O2 -Wall -g -fPIC $(INCLUDES) $(DEFINES)

# by default, the plugin is copied in ~/.qi/ so that Qi can load it automatically.
all: $(LIB) #$(DIST_LIB)

$(DIST_LIB): $(LIB)
	install -s -m 644 $< $@

install: $(DIST_LIB)

$(LIB): $(OBJS) #Makefile
	#$(CC) -shared -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(LOAD_PATH)$@ $<

clean:
	rm -f *.o *.so *~
