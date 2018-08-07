include ../../config.mak

# modify to get the path of the qemacs sources
QEMACS_PATH=../../
LOAD_PATH=../
# define the object files it contains
OBJS=charsetmore.o charset_table.o

ifdef CONFIG_ARABIC
OBJS+=arabic.o
endif

ifdef CONFIG_DEVANAGARI
OBJS+=indic.o
endif

INCLUDES=-I$(QEMACS_PATH)
DEFINES=-DQE_MODULE
CFLAGS+= -g -fPIC $(INCLUDES) $(DEFINES)

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(LOAD_PATH)$@ $<

clean:
	rm -f $(LOAD_PATH)charsetmore.o \
		$(LOAD_PATH)charset_table.o \
		$(LOAD_PATH)arabic.o \
		$(LOAD_PATH)indic.o \
