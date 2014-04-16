
cyclus_root = /home/r/cyc/install
modules = recipe_extractor recipe_mixer separator animal

##### dependency config #####
includes += -I$(cyclus_root)/include/cyclus
includes += $(shell pkg-config --cflags sqlite3)
includes += $(shell pkg-config --cflags cbc)
includes += $(shell pkg-config --cflags libxml-2.0)
includes += $(shell pkg-config --cflags libxml++-2.6)

libs += -L$(cyclus_root)/lib -lcycluscore

##### variable setup #####
cycpp = $(cyclus_root)/bin/cycpp.py
source = $(wildcard *.cc)
source_unique = $(sort $(source))
libnames = $(modules:%=lib%.so)

objs_tmp = $(source_unique:%.cc=%.o)
module_tmp = $(modules:%=%.o)
objs = $(filter-out $(module_tmp),$(objs_tmp))

##### primary targets #####
all: $(libnames)
	ls | grep 'lib[a-z].\+\.so' | xargs rm -f

clean:
	rm -f *.o *.d
	rm -f *.so
	rm -f *.gen.c*
	rm -f *.gen.h*

##### pattern rules to build all object files from source #####

lib%.so : %.foo $(objs)
	@echo "CXX -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $(objs) $< -o $@"
	@$(CXX) -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $(objs) $< -o $@
	@cp $@ `echo $@ | sed 's/_\(.\)/\u\1/' | sed 's/lib\(.\)\(.*\)\.so/lib\u\1\2.so/'`

%.o : %.cc %.h
	@echo "CXX -c -fpic $(CXXFLAGS) $(CPPFLAGS) $< -o $@"
	@$(CXX) -c -fpic $(CXXFLAGS) $(CPPFLAGS) $(includes) $< -o $@

%.foo : %.gen.cc %.gen.h
	@echo "CXX -c -fpic $(CXXFLAGS) $(CPPFLAGS) $< -o $o"
	@$(CXX) -c -fpic $(CXXFLAGS) $(CPPFLAGS) $(includes) $< -o $@

%.gen.cc: %.cc
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 
	sed -i 's/$*.h/$*.gen.h/' $@

%.gen.h: %.h
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 

# disable makes built-in implicit rules
.SUFFIXES:
