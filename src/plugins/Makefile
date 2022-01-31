LOAD_PATH=../../build

all: core extra

test:
	@echo "======================================== Core tests == "
	make -C hex test
	make -C list test
	make -C tty test
#	make -C shell test
	
core:
	@echo "======================================== Core plugins == "
	make -C hex all
	make -C list all
	make -C tty all
#	make -C shell all
# make the group header file
	find ./ -name "*.h" -exec cat {} >> ./plugincore.g \;
# rename it
	mv plugincore.g plugincore.h

# ar rcs libplugincore.a *.o
	ar -cvq libplugincore.a *.o

	mv ./libplugincore.a $(LOAD_PATH)/$< 
	rm *.o

extra:
	@echo "======================================== Extra plugins == "
	#buffers
	make -C bufed all
# direcotry browing
	make -C dired all
# basic c language stuff (java, javascript, etc too)
	make -C clang all
# hex mode for unicode files
	make -C unihex all
# xml (and xml based languges)
	make -C xml all
	make -C charsetmore all
# latex mode
	make -C latex-mode all
# markdown mode
	make -C markdown-mode all
	make -C html all
# just and example plugin
	make -C example all
#	make -C php-mode all
	make -C css-mode all
# make -C quick-open all
	make -C python-mode all
	make -C config-mode all
	
# ar rcs libplugins.a *.o
	ar -cvq libplugins.a *.o

	mv ./libplugins.a $(LOAD_PATH)/$< 
	rm *.o

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
	make -C html clean
	make -C php-mode clean
	make -C css-mode clean
# make -C quick-open clean
	make -C python-mode clean
	
	rm -f $(LOAD_PATH)/libplugincore.a
	rm -f $(LOAD_PATH)/libplugins.a
	rm -f plugincore.h
