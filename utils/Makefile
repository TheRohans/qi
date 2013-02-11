include ../src/config.mak

all: banner_help 
	

kmaptoqe$(EXE): kmaptoqe.c
	$(HOST_CC) $(CFLAGS) -o $@ $<

fbftoqe$(EXE): fbftoqe.c
	$(CC) $(CFLAGS) -o $@ $<

ligtoqe$(EXE): ligtoqe.c
	$(HOST_CC) $(CFLAGS) -o $@ $<

cptoqe$(EXE): cptoqe.c
	$(HOST_CC) $(CFLAGS) -o $@ $<

##############################################################

clean:
	rm -f *.o *.exe
	rm -f kmaptoqe$(EXE) fbftoqe$(EXE) ligtoqe$(EXE) cptoqe$(EXE)

banner_help:
	@echo '=================================================================='
	@echo '=                             气                                 ='
	@echo '=                           qi utils                             ='
	@echo '=================================================================='
	@echo ''
	@echo 'Welcome to qi.'
	@echo ''
	@echo 'make kmaptoqe'
	@echo 'make fbftoqe'
	@echo 'make ligtoqe'
	@echo 'make cptoqe'
