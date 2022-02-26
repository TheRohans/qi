LOAD_PATH=../../build

all: core extra

test:
	@echo "======================================== Core tests == "
	make -C hex test
	make -C list test
	make -C tty test
	
core:
	@echo "======================================== Core plugins == "
	make -C hex all
	make -C list all
	make -C tty all
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
# buffer browser
	make -C bufed all
# direcotry browsing
	make -C dired all
# basic c language stuff (java, javascript, go, terraform, etc too)
	make -C clang all
# hex mode for unicode files
	make -C unihex all
# xml (and xml based languges)
	make -C xml all
	make -C html all
	make -C css-mode all
# latex mode
	make -C latex-mode all
# markdown mode
	make -C markdown-mode all
# just and example plugin
	make -C example all
	make -C python-mode all
# Makefile, yaml, ini, etc
	make -C config-mode all
# 
	make -C golang all
	make -C tslang all
	
# ar rcs libplugins.a *.o
	ar -cvq libplugins.a *.o

	mv ./libplugins.a $(LOAD_PATH)/$< 
	rm *.o

clean:
	@echo "======================================== Cleaning all core plugins == "
	make -C hex clean
	make -C list clean
	make -C tty clean
	@echo "======================================== Cleaning all extra plugins == "
	make -C bufed clean
	make -C dired clean
	make -C clang clean
	make -C unihex clean
	make -C xml clean
	make -C latex-mode clean
	make -C markdown-mode clean
	make -C html clean
	make -C css-mode clean
	make -C python-mode clean
	make -C golang clean
	make -C tslang clean
	
	rm -f $(LOAD_PATH)/libplugincore.a
	rm -f $(LOAD_PATH)/libplugins.a
	rm -f plugincore.h

