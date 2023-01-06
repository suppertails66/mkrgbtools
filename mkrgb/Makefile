IDIR := src
SRCDIR := src
ODIR := obj
LDIR :=

BLACKTDIR = ../blackt
LIBGBDIR = ../libgb

AR=ar
CXX=g++
# Compile only
CDEFINES = -DBLACKT_ENABLE_LIBPNG
#CLIBS = -lpng
CFLAGS = -std=gnu++11 -O2 -Wall -L${LIBGBDIR} -lgb -L${BLACKTDIR} -lblackt -lpng
CINCLUDES = -I${BLACKTDIR}/src -I${LIBGBDIR}/src
CXXFLAGS=$(CFLAGS) $(CDEFINES) $(CINCLUDES) -I$(IDIR)

LIBDEPS := $(LIBGBDIR)/libgb.a $(BLACKTDIR)/libblackt.a

TOOLSRCS := $(wildcard $(SRCDIR)/*.cpp)
TOOLSINDIR := $(patsubst %.cpp,%,$(TOOLSRCS))
TOOLS := $(notdir $(TOOLSINDIR))

.SECONDEXPANSION:

all: $(BLACKTDIR)/libblackt.a $(LIBGBDIR)/libgb.a $(TOOLS)

blackt:
	$(MAKE) -C${BLACKTDIR}

libgb:
	$(MAKE) -C${LIBGBDIR}

$(BLACKTDIR)/libblackt.a: $(BLACKTDIR)/src/**/*.cpp
	$(MAKE) -C ${BLACKTDIR} all

$(LIBGBDIR)/libgb.a: $(LIBGBDIR)/src/**/*.cpp
	$(MAKE) -C ${LIBGBDIR} all

$(TOOLS): $(SRCDIR)/$$@.cpp $(LIBDEPS)
	$(MAKE) blackt
	$(MAKE) libgb
	$(CXX) $(SRCDIR)/$@.cpp $(OBJ) -o $(notdir $@) $(CXXFLAGS)

.PHONY: blackt libgb cleanme clean

cleanme:
	rm -f $(TOOLS)

clean: cleanme
#	rm -f $(LIB)
#	rm -rf $(ODIR)
	$(MAKE) -C ${BLACKTDIR} clean
	$(MAKE) -C ${LIBGBDIR} clean
