include configure.in

ifndef OCTAVE_FORGE
	MKOCTFILE=mkoctfile
endif

SRC=__mosek__.cc
PROGS=__mosek__.oct

all: $(PROGS)

__mosek__.oct: $(SRC)
	$(MKOCTFILE) -v $(SRC) -I"$(PKG_MOSEKHOME)/h" "-L\"$(PKG_MOSEKHOME)/bin\"" -l$(PKG_MOSEKLIB)

clean: ; $(RM) *.o core octave-core *.oct *~
