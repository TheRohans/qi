# modify to get the path of the qi sources
QEMACS_PATH=../../
LOAD_PATH=../
# define the object files it contains
OBJS=css-mode.o

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
CFLAGS+= -g -fPIC $(INCLUDES) $(DEFINES)

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(LOAD_PATH)$@ $<

clean:
	rm -f $(LOAD_PATH)css-mode.o
