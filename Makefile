.PHONY: doc

all: banner_help 

qi:
	@echo '======================================== Building qi == '
	mkdir -p build
	make -C src all

afl:
	@echo '========================================= Fuzzing qi == '
# TODO: Currently this just fails with WARNING: Test case results in a time out
# I think this is because the application need to exit right after it loads
# a file for testing, but currently it just stays in the game loop
ifneq (, $(shell which afl-gcc))
	./configure --cc=afl-gcc # --enable-debug
	make clean qi
ifneq (, $(shell which afl-fuzz))
	export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1; \
	export AFL_SKIP_CPUFREQ=1; \
	export AFL_NO_FORKSRV=1; \
		afl-fuzz -t 2000+ -i $(PWD)/test_data/ -o build/fuzz $(PWD)/qi @@
endif
endif

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
	rm -rf doc/output

doc:
ifneq (, $(shell which doxygen))
	doxygen doc/doxygen/Doxyfile
endif

checks:
ifneq (, $(shell which cppcheck))
	cppcheck --enable=all --suppress=missingIncludeSystem .
endif


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
