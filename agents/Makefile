
cyclus_root = /home/r/cyc/install
modules = recipe_extractor recipe_mixer separator
CXXFLAGS += -g

##### dependency config #####
includes += -I$(cyclus_root)/include/cyclus
includes += -I$(HOME)/.local/include
includes += $(shell pkg-config --cflags sqlite3)
includes += $(shell pkg-config --cflags cbc)
includes += $(shell pkg-config --cflags libxml-2.0)
includes += $(shell pkg-config --cflags libxml++-2.6)

libs += -L$(cyclus_root)/lib
libs += -L$(HOME)/.local/lib
libs += -lcycluscore -lkitlus

##### variable setup #####
cycpp = $(cyclus_root)/bin/cycpp.py
libnames = $(modules:%=lib%.so)

##### primary targets #####
all: $(libnames)
	ls | grep 'lib[a-z].\+\.so' | xargs rm -f

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.gen.c*
	rm -f *.gen.h*

##### pattern rules to build all object files from source #####

lib%.so : %.o
	@echo "CXX -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $< -o $@"
	@$(CXX) -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $< -o $@
	@cp $@ `echo $@ | sed 's/_\(.\)/\u\1/' | sed 's/lib\(.\)\(.*\)\.so/lib\u\1\2.so/'`

%.o : %.gen.cc %.gen.h
	@echo "CXX -c -fpic $(CXXFLAGS) $(CPPFLAGS) $< -o $o"
	@$(CXX) -c -fpic $(CXXFLAGS) $(CPPFLAGS) $(includes) $< -o $@

%.gen.cc: %.cc
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 
	sed -i 's/$*.h/$*.gen.h/' $@

%.gen.h: %.h
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 

.PRECIOUS: %.gen.h %.gen.cc %.o

# disable makes built-in implicit rules
.SUFFIXES:
