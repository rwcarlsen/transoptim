
src = $(filter-out %.gen.cc,$(wildcard *.cc))
$(info "src: $(src)")
libname=librwc.so

##### set default var vals #####
ifeq ($(strip $(PREFIX)),)
	PREFIX = $(HOME)/.local
endif

ifeq ($(strip $(CYCLUS_ROOT)),)
ifneq ($(strip $(shell which cyclus)),)
CYCLUS_ROOT = $(strip $(shell cyclus --install-path))
else
CYCLUS_ROOT = $(HOME)/.local
endif
endif

LDFLAGS +="-Wl,-rpath,$(CYCLUS_ROOT)/lib,-rpath,$(HOME)/.local/lib"

##### dependency config #####
includes += -I$(HOME)/.local/include
includes += -I$(CYCLUS_ROOT)/include/cyclus
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags sqlite3)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags cbc)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags libxml-2.0)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags libxml++-2.6)

libs += -L$(CYCLUS_ROOT)/lib
libs += -L$(HOME)/.local/lib
libs += -lcyclus -lkitlus

##### variable setup #####
cycpp = $(CYCLUS_ROOT)/bin/cycpp.py
objs = $(src:%.cc=%.o)

prefix = $(DESTDIR)/$(PREFIX)
ifeq ($(strip $(DESTDIR)),)
	prefix = $(PREFIX)
endif

##### primary targets #####
build: $(libname)

$(libname) : $(objs)
	@echo "CXX -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $^ -o $@"
	@$(CXX) -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(libs) $^ -o $@

debug: CXXFLAGS += -g
debug: build

release: CXXFLAGS += -O3
release: build

install: release
	@mkdir -p $(prefix)/lib/cyclus
	cp *.so $(prefix)/lib/cyclus/

clean:
	rm -f *.o *.so
	rm -f *.gen.c*
	rm -f *.gen.h*
	rm -f cyclus.sqlite *.dat *.svg

predprey: pred-prey.xml
	@rm -f cyclus.sqlite
	cyclus -v3 --flat-schema $<
	cycpost cyclus.sqlite
	metric -db cyclus.sqlite deployseries Rabbit > rabbit.dat
	metric -db cyclus.sqlite deployseries Wolf > wolf.dat
	gnuplot pred-prey.plot

##### pattern rules to build all object files from source #####

%.o : %.gen.cc %.gen.h
	@echo "CXX -c -fpic $(CXXFLAGS) $(CPPFLAGS) $< -o $o"
	@$(CXX) -c -fpic $(CXXFLAGS) $(CPPFLAGS) $(includes) $< -o $@

%.gen.cc: %.cc
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@python2 $(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 
	sed -i 's/$*.h/$*.gen.h/' $@

%.gen.h: %.h
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@python2 $(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 

.PRECIOUS: %.gen.h %.gen.cc

# disable makes built-in implicit rules
.SUFFIXES:
