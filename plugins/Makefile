LOAD_PATH=../build

all: core

core:
	@echo " == Building all core plugins == "
	make -C example all
	make -C clang all
	# make the group header file
	find ./ -name "*.h" -exec cat {} >> ./plugincore.g \;
	# rename it
	mv plugincore.g plugincore.h
	ar rcs libplugincore.a *.o
	mv ./libplugincore.a $(LOAD_PATH)/$< 
	rm *.o
	@echo " == Done building all core plugins == "

#install: 
#	make -C example install

clean:
	@echo " == Cleaning all core plugins == "
	make -C example clean
	make -C clang clean
	rm -f $(LOAD_PATH)/libplugincore.a
	rm -f plugincore.h

# NOTES:
#	gcc -c -o out.o out.c
#	-c means to create an intermediary object file, rather than an executable.
#	ar rcs libout.a out.o
# lipo -info yourlib.a
# nm yourlib.a | grep the_symbol_without_args