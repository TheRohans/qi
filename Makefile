
all: banner_help 

qi:
	make -C src all
	
clean:
	make -C src clean

distclean: clean
	rm -f config.h config.mak

test:
	make -C tests test

banner_help:
	@echo '=================================================================='
	@echo '=                             气                                 ='
	@echo '=                             qi                                 ='
	@echo '=================================================================='
	@echo ''
	@echo 'Welcome to qi.'
	@echo ''
	@echo 'To build run ./configure, and then you can do one of the '
	@echo ' following:'
	@echo ''
	@echo 'make qi        - kick off script to make the edtior (probably what'
	@echo '                  you are after)'
	@echo 'make clean     - clean up build files'
	@echo 'make distclean - clean build and config files (aka start over)'
	@echo ''
