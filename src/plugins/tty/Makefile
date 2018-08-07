# modify to get the path of the qemacs sources
QEMACS_PATH=../../
LOAD_PATH=../

# define the object files it contains
OBJS=tty.o

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
CFLAGS+= -g -fPIC $(INCLUDES) $(DEFINES)

TEST_SRC=$(wildcard *_test.c)

all: $(OBJS) moveo

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

moveo: 
	mv *.o $(LOAD_PATH)
	

clean:
	rm -f $(LOAD_PATH)tty.o

#############################################################################

test:
	@echo 'tty has no tests'

