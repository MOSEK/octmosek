include configure.in

ifndef OCTAVE_FORGE
	MKOCTFILE=mkoctfile
endif

SRC=OctMOSEK.cc omsk_msg_base.cc omsk_msg_mosek.cc omsk_obj_arguments.cc omsk_obj_constraints.cc omsk_obj_mosek.cc omsk_utils_interface.cc omsk_utils_mosek.cc omsk_utils_octave.cc
PROGS=__mosek__.oct

all: $(PROGS)

__mosek__.oct: $(SRC)
	$(MKOCTFILE) -v -Wall -o __mosek__.oct $(SRC) -I"$(PKG_MOSEKHOME)/h" "-L\"$(PKG_MOSEKHOME)/bin\"" -l$(PKG_MOSEKLIB)

clean: ; $(RM) *.o core octave-core *.oct *~
