LOAD_PATH=../../build

all: core extra

core:
	@echo "======================================== Core plugins == "
	make -C hex all
	make -C list all
	make -C tty all
	make -C shell all
	# make the group header file
	find ./ -name "*.h" -exec cat {} >> ./plugincore.g \;
	# rename it
	mv plugincore.g plugincore.h
	ar rcs libplugincore.a *.o
	mv ./libplugincore.a $(LOAD_PATH)/$< 
	rm *.o

extra:
	@echo "======================================== Extra plugins == "
	#buffers
	make -C bufed all
	#direcotry browing
	make -C dired all
	#basic c language stuff (java, javascript, etc too)
	make -C clang all
	#hex mode for unicode files
	make -C unihex all
	#xml (and xml based languges)
	make -C xml all
	make -C charsetmore all
	#latex, don't much care about this...
	make -C latex-mode all
	#markdown mode
	make -C markdown-mode all
	#just and example plugin
	make -C example all
	
	ar rcs libplugins.a *.o
	mv ./libplugins.a $(LOAD_PATH)/$< 
	rm *.o

	

#install: 
#	make -C example install

clean:
	@echo "======================================== Cleaning all core plugins == "
	make -C hex clean
	make -C list clean
	make -C tty clean
	make -C shell clean
	@echo "======================================== Cleaning all extra plugins == "
	make -C bufed clean
	make -C dired clean
	make -C clang clean
	make -C unihex clean
	make -C xml clean
	make -C charsetmore clean
	make -C latex-mode clean
	make -C markdown-mode clean
	
	rm -f $(LOAD_PATH)/libplugincore.a
	rm -f $(LOAD_PATH)/libplugins.a
	rm -f plugincore.h

# NOTES:
#	gcc -c -o out.o out.c
#	-c means to create an intermediary object file, rather than an executable.
#	ar rcs libout.a out.o
# lipo -info yourlib.a
# nm yourlib.a | grep the_symbol_without_args