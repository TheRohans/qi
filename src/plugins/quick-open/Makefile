QEMACS_PATH=../../
LOAD_PATH=../
OBJS=quick-open.o

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
CFLAGS+= -g -fPIC $(INCLUDES) $(DEFINES)

all: $(OBJS)

%.o: %.c
	$(CC) -I../ $(CFLAGS) -c -o $(LOAD_PATH)$@ $<

clean:
	rm -f $(LOAD_PATH)quick-open.o
