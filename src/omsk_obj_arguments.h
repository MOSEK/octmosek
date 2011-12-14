#ifndef OMSK_OBJ_ARGUMENTS_H_
#define OMSK_OBJ_ARGUMENTS_H_

#include "omsk_msg_mosek.h"
#include "omsk_obj_mosek.h"

#include "omsk_obj_constraints.h"

#include <octave/oct.h>
#include <octave/ov-struct.h>

#include <string>
#include <vector>

struct options_type {
private:
	bool initialized;

public:
	// Recognised options arguments in Octave
	// TODO: Upgrade to new C++11 initialisers
	static const struct OCT_ARGS_type {

		std::vector<std::string> arglist;
		const std::string useparam;
		const std::string usesol;
		const std::string verbose;
		const std::string writebefore;
		const std::string writeafter;

		OCT_ARGS_type() :
			useparam("useparam"),
			usesol("usesol"),
			verbose("verbose"),
			writebefore("writebefore"),
			writeafter("writeafter")
		{
			std::string temp[] = {useparam, usesol, verbose, writebefore, writeafter};
			arglist = std::vector<std::string>(temp, temp + sizeof(temp)/sizeof(std::string));
		}
	} OCT_ARGS;


	// Data definition
	bool	useparam;
	bool 	usesol;
	double 	verbose;
	std::string	writebefore;
	std::string	writeafter;

	// Default values of optional arguments
	options_type();

	// Read options from Octave (write not implemented)
	void OCT_read(Octave_map &arglist);
};


class problem_type {
private:
	bool initialized;

public:

	//
	// Recognised problem arguments in Octave
	// TODO: Upgrade to new C++11 initialisers
	//
	static const struct OCT_ARGS_type {
	public:
		std::vector<std::string> arglist;
		const std::string sense;
		const std::string c;
		const std::string c0;
		const std::string A;
		const std::string blc;
		const std::string buc;
		const std::string blx;
		const std::string bux;
		const std::string cones;
		const std::string intsub;
		const std::string sol;
		const std::string iparam;
		const std::string dparam;
		const std::string sparam;
//		const std::string options;

		OCT_ARGS_type() :
			sense("sense"),
			c("c"),
			c0("c0"),
			A("A"),
			blc("blc"),
			buc("buc"),
			blx("blx"),
			bux("bux"),
			cones("cones"),
			intsub("intsub"),
			sol("sol"),
			iparam("iparam"),
			dparam("dparam"),
			sparam("sparam")
//			options("options")
		{
			std::string temp[] = {sense, c, c0, A, blc, buc, blx, bux, cones, intsub, sol, iparam, dparam, sparam}; //options
			arglist = std::vector<std::string>(temp, temp + sizeof(temp)/sizeof(std::string));
		}

	} OCT_ARGS;

	//
	// Data definition (intentionally kept close to Octave types)
	// Note: 'MSKintt' is compliant with 'octave_idx_type', but 'MSKint64t' would not be.
	//
	MSKintt numnz;
	MSKintt	numcon;
	MSKintt	numvar;
	MSKintt	numintvar;
	MSKintt	numcones;

	MSKobjsensee	sense;
	RowVector 		c;
	double 			c0;
	SparseMatrix	A;
	RowVector 		blc;
	RowVector 		buc;
	RowVector 		blx;
	RowVector 		bux;
	conicSOC_type 	cones;
	int32NDArray 	intsub;
	Octave_map 		initsol;
	Octave_map 		iparam;
	Octave_map 		dparam;
	Octave_map	 	sparam;
	options_type 	options;

	// Default values of optional arguments
	problem_type();

	// Read and write problem description from and to Octave
	void OCT_read(Octave_map &arglist);
	void OCT_write(Octave_map &prob_val);

	// Read and write problem description from and to MOSEK
	void MOSEK_read(Task_handle &task);
	void MOSEK_write(Task_handle &task);
};

#endif /* OMSK_OBJ_ARGUMENTS_H_ */
