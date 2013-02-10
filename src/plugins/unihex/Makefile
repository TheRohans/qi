# modify to get the path of the qemacs sources
QEMACS_PATH=../../
LOAD_PATH=../
# define the object files it contains
OBJS=unihex.o

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
CC=gcc
CFLAGS=-O2 -Wall -g -fPIC $(INCLUDES) $(DEFINES)

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(LOAD_PATH)$@ $<

clean:
	rm -f $(LOAD_PATH)unihex.o
