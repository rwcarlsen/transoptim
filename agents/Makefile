
src = $(wildcard *.cc)
libname=librwc.so

##### set default var vals #####
ifeq ($(strip $(CYCLUS_ROOT)),)
	CYCLUS_ROOT = $(HOME)/.local $(HOME)/cyc/install /usr/local /usr
endif
ifeq ($(strip $(PREFIX)),)
	PREFIX = $(HOME)/.local
endif

##### dependency config #####
includes += -I$(HOME)/.local/include
includes += $(CYCLUS_ROOT:%=-I%/include/cyclus)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags sqlite3)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags cbc)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags libxml-2.0)
includes += $(shell export PKG_CONFIG_PATH="$(HOME)/.local/lib/pkgconfig"; pkg-config --cflags libxml++-2.6)

libs += $(CYCLUS_ROOT:%=-L%/lib)
libs += -L$(HOME)/.local/lib
libs += -lcyclus -lkitlus

##### variable setup #####
cycpp = cycpp.py
objs = $(src:%.cc=%.o)

prefix = $(DESTDIR)/$(PREFIX)
ifeq ($(strip $(DESTDIR)),)
	prefix = $(PREFIX)
endif

##### primary targets #####
build: $(libname)

$(libname) : $(objs)
	@echo "CXX -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $^ -o $@"
	@$(CXX) -shared -fpic -rdynamic $(CXXFLAGS) $(CPPFLAGS) $(libs) $^ -o $@

debug: CXXFLAGS += -g
debug: build

release: CXX_FLAGS += -O3
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
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 
	sed -i 's/$*.h/$*.gen.h/' $@

%.gen.h: %.h
	@echo "cycpp --cpp-path=$(CXX) --pass3-use-orig $< -o $@"
	@$(cycpp) --cpp-path=$(CXX) --pass3-use-orig $(includes) $< -o $@ 

.PRECIOUS: %.gen.h %.gen.cc

# disable makes built-in implicit rules
.SUFFIXES:
