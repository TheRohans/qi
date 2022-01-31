.PHONY: doc

all: banner_help 

qi:
	@echo '======================================== Building qi == '
	mkdir -p build
	make -C src all
	
clean:
	@echo '======================================== Cleaning qi == '
	make -C src clean
	rm -rf build
	rm -rf dist

test:
	@echo '======================================= Running Tests == '
	make -C src test

dist: clean qi doc
	@echo '======================================= Making Distro == '
	mkdir -p dist
	mv qi dist/qi
	mv doc/output dist/doc

distclean: clean
	rm -f config.h config.mak
	rm -rf dist

doc:
	doxygen doc/doxygen/Doxyfile

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
	@echo 'make doc       - run doxygen on the code base'
	@echo ''
