LOAD_PATH=../../build

MAKE?=$(MAKE)

all: core extra

test:
	@echo "======================================== Core tests == "
	$(MAKE) -C hex test
	$(MAKE) -C list test
	$(MAKE) -C tty test
	
core:
	@echo "======================================== Core plugins == "
	$(MAKE) -C hex all
	$(MAKE) -C list all
	$(MAKE) -C tty all
# $(MAKE) the group header file
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
	$(MAKE) -C bufed all
# direcotry browsing
	$(MAKE) -C dired all
# basic c language stuff (java, javascript, go, terraform, etc too)
	$(MAKE) -C clang all
# hex mode for unicode files
	$(MAKE) -C unihex all
# xml (and xml based languges)
	$(MAKE) -C xml all
	$(MAKE) -C html all
	$(MAKE) -C css-mode all
# latex mode
	$(MAKE) -C latex-mode all
# markdown mode
	$(MAKE) -C markdown-mode all
# just and example plugin
	$(MAKE) -C example all
	$(MAKE) -C python-mode all
# Makefile, yaml, ini, etc
	$(MAKE) -C config-mode all
# 
	$(MAKE) -C golang all
	$(MAKE) -C tslang all
	$(MAKE) -C maths-mode all
	
# ar rcs libplugins.a *.o
	ar -cvq libplugins.a *.o

	mv ./libplugins.a $(LOAD_PATH)/$< 
	rm *.o

clean:
	@echo "======================================== Cleaning all core plugins == "
	$(MAKE) -C hex clean
	$(MAKE) -C list clean
	$(MAKE) -C tty clean
	@echo "======================================== Cleaning all extra plugins == "
	$(MAKE) -C bufed clean
	$(MAKE) -C dired clean
	$(MAKE) -C clang clean
	$(MAKE) -C unihex clean
	$(MAKE) -C xml clean
	$(MAKE) -C latex-mode clean
	$(MAKE) -C markdown-mode clean
	$(MAKE) -C html clean
	$(MAKE) -C css-mode clean
	$(MAKE) -C python-mode clean
	$(MAKE) -C golang clean
	$(MAKE) -C tslang clean
	$(MAKE) -C maths-mode clean
	
	rm -f $(LOAD_PATH)/libplugincore.a
	rm -f $(LOAD_PATH)/libplugins.a
	rm -f plugincore.h

