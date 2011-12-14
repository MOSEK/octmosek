#include "omsk_obj_arguments.h"

#include "omsk_utils_octave.h"
#include "omsk_utils_mosek.h"

#include <string>

using std::string;

// ------------------------------
// Class options_type
// ------------------------------

const options_type::OCT_ARGS_type options_type::OCT_ARGS;

// Default values of optional arguments
options_type::options_type() :
	initialized(false),

	useparam(true),
	usesol(true),
	verbose(10),
	writebefore(""),
	writeafter("")
{}

void options_type::OCT_read(Octave_map &arglist) {
	printdebug("Reading options");

	// Read verbose and update message system
	map_seek_Scalar(&verbose, arglist, OCT_ARGS.verbose, true);
	mosek_interface_verbose = verbose;
	printpendingmsg();

	// Read remaining input arguments
	map_seek_Boolean(&useparam, arglist, OCT_ARGS.useparam,  true);
	map_seek_Boolean(&usesol, arglist, OCT_ARGS.usesol, true);
	map_seek_String(&writebefore, arglist, OCT_ARGS.writebefore, true);
	map_seek_String(&writeafter, arglist, OCT_ARGS.writeafter, true);

	// Check for bad arguments
	validate_OctaveMap(arglist, "", OCT_ARGS.arglist);

	initialized = true;
}


// ------------------------------
// Class problem_type
// ------------------------------

const problem_type::OCT_ARGS_type problem_type::OCT_ARGS;

// Default values of optional arguments
problem_type::problem_type() :
	initialized(false),

	sense	(MSK_OBJECTIVE_SENSE_UNDEFINED),
	c0		(0),
	options	(options_type())
{}


void problem_type::OCT_read(Octave_map &arglist) {
	if (initialized) {
		throw msk_exception("Internal error in problem_type::OCT_read, a problem was already loaded");
	}
	printdebug("Started reading Octave problem input");

	// Constraint Matrix
	map_seek_SparseMatrix(&A, arglist, OCT_ARGS.A);
	numnz = A.nelem();
	numcon = A.dimensions(0);
	numvar = A.dimensions(1);

	// Objective sense
	string sensename = "UNDEFINED";
	map_seek_String(&sensename, arglist, OCT_ARGS.sense);
	sense = get_mskobjective(sensename);

	// Objective function
	map_seek_RowVector(&c, arglist, OCT_ARGS.c);			validate_RowVector(c, OCT_ARGS.c, numvar);
	map_seek_Scalar(&c0, arglist, OCT_ARGS.c0, true);

	// Constraint and Variable Bounds
	map_seek_RowVector(&blc, arglist, OCT_ARGS.blc);		validate_RowVector(blc, OCT_ARGS.blc, numcon);
	map_seek_RowVector(&buc, arglist, OCT_ARGS.buc);		validate_RowVector(buc, OCT_ARGS.buc, numcon);
	map_seek_RowVector(&blx, arglist, OCT_ARGS.blx);		validate_RowVector(blx, OCT_ARGS.blx, numvar);
	map_seek_RowVector(&bux, arglist, OCT_ARGS.bux);		validate_RowVector(bux, OCT_ARGS.bux, numvar);

	// Cones
	Cell objcones;	map_seek_Cell(&objcones, arglist, OCT_ARGS.cones, true);
	cones.OCT_read(objcones);
	numcones = cones.numcones;

	// Integers variables and initial solutions
	map_seek_IntegerArray(&intsub, arglist, OCT_ARGS.intsub, true);
	map_seek_OctaveMap(&initsol, arglist, OCT_ARGS.sol, true);
	numintvar = intsub.nelem();

	// Parameters
	map_seek_OctaveMap(&iparam, arglist, OCT_ARGS.iparam, true);
	map_seek_OctaveMap(&dparam, arglist, OCT_ARGS.dparam, true);
	map_seek_OctaveMap(&sparam, arglist, OCT_ARGS.sparam, true);

//		// Options (use this to allow options to overwrite other input channels)
//		Octave_map options_arglist;
//		map_seek_OctaveMap(&options_arglist, arglist, OCT_ARGS.options, true);
//		if (options_arglist.nfields() != 0) {
//			options.OCT_read(options_arglist);
//		}

	// Check for bad arguments
	validate_OctaveMap(arglist, "", OCT_ARGS.arglist);

	initialized = true;
}


void problem_type::OCT_write(Octave_map &prob_val) {
	if (!initialized) {
		throw msk_exception("Internal error in problem_type::OCT_write, no problem was loaded");
	}
	printdebug("Started writing Octave problem output");

	// Objective sense
	prob_val.assign("sense", octave_value(get_objective(sense), '\"'));

	// Objective
	prob_val.assign("c", octave_value(c));
	prob_val.assign("c0", octave_value(c0));

	// Constraint Matrix A
	prob_val.assign("A", octave_value(A));

	// Constraint and variable bounds
	prob_val.assign("blc", octave_value(blc));
	prob_val.assign("buc", octave_value(buc));
	prob_val.assign("blx", octave_value(blx));
	prob_val.assign("bux", octave_value(bux));

	// Cones
	if (numcones > 0) {
		Cell objcones;	cones.OCT_write(objcones);
		prob_val.assign("cones", octave_value(objcones));
	}

	// Integer subindexes
	if (numintvar > 0) {
		prob_val.assign("intsub", octave_value(intsub));
	}

	// Parameters
	if (options.useparam) {
		if (!isEmpty(iparam))
			prob_val.assign("iparam", octave_value(iparam));

		if (!isEmpty(dparam))
			prob_val.assign("dparam", octave_value(dparam));

		if (!isEmpty(sparam))
			prob_val.assign("sparam", octave_value(sparam));
	}

	// Initial solution
	if (options.usesol) {
		if (!isEmpty(initsol))
			prob_val.assign("sol", octave_value(initsol));
	}
}


void problem_type::MOSEK_read(Task_handle &task) {
	if (initialized) {
		throw msk_exception("Internal error in problem_type::MOSEK_read, a problem was already loaded");
	}
	printdebug("Started reading MOSEK problem output");

	// Get problem dimensions
	{
		errcatch( MSK_getnumanz(task, &numnz) );
		errcatch( MSK_getnumcon(task, &numcon) );
		errcatch( MSK_getnumvar(task, &numvar) );
		errcatch( MSK_getnumintvar(task, &numintvar) );
		errcatch( MSK_getnumcone(task, &numcones) );
	}

	// Objective sense and constant
	{
		printdebug("problem_type::MOSEK_read - Objective sense and constant");
		errcatch( MSK_getobjsense(task, &sense) );
		errcatch( MSK_getcfix(task, &c0) );
	}

	// Objective coefficients
	{
		printdebug("problem_type::MOSEK_read - Objective coefficients");

		c = RowVector(numvar);
		double *pc = c.fortran_vec();
		errcatch( MSK_getc(task, pc) );
	}

	// Constraint Matrix A
	{
		printdebug("problem_type::MOSEK_read - Constraint matrix");

		A = SparseMatrix(numcon, numvar, numnz);
		MSKintt *ptrb = A.cidx();
		MSKlidxt *sub = A.ridx();
		MSKrealt *val = A.data();
		MSKintt surp[1] = {numnz};

		errcatch( MSK_getaslice(task, MSK_ACC_VAR, 0, numvar, numnz, surp,
				ptrb, ptrb+1, sub, val) );
	}

	// Constraint bounds
	{
		printdebug("problem_type::MOSEK_read - Constraint bounds");

		blc = RowVector(numcon);
		buc = RowVector(numcon);

		double *pblc = blc.fortran_vec();
		double *pbuc = buc.fortran_vec();

		get_boundvalues(task, pblc, pbuc, MSK_ACC_CON, numcon);
	}

	// Variable bounds
	{
		printdebug("problem_type::MOSEK_read - Variable bounds");

		blx = RowVector(numvar);
		bux = RowVector(numvar);

		double *pblx = blx.fortran_vec();
		double *pbux = bux.fortran_vec();

		get_boundvalues(task, pblx, pbux, MSK_ACC_VAR, numvar);
	}

	// Cones
	if (numcones > 0) {
		printdebug("problem_type::MOSEK_read - Cones");
		cones.MOSEK_read(task);
	}

	// Integer subindexes
	if (numintvar > 0) {
		printdebug("problem_type::MOSEK_read - Integer subindexes");

		intsub = int32NDArray(dim_vector(1,numintvar));
		octave_int32 *pintsub = intsub.fortran_vec();

		int idx = 0;
		MSKvariabletypee type;
		for (int i=0; i<numvar; i++) {
			errcatch( MSK_getvartype(task,i,&type) );

			// Octave indexes count from 1, not from 0 as MOSEK
			if (type == MSK_VAR_TYPE_INT) {
				pintsub[idx++] = octave_int32(i+1);

				if (idx >= numintvar)
					break;
			}
		}
	}

	// Integer Parameters
	if (options.useparam) {
		printdebug("problem_type::MOSEK_read - Integer Parameters");

		iparam = Octave_map();
		get_int_parameters(iparam, task);
	}

	// Double Parameters
	if (options.useparam) {
		printdebug("problem_type::MOSEK_read - Double Parameters");

		dparam = Octave_map();
		get_dou_parameters(dparam, task);
	}

	// String Parameters
	if (options.useparam) {
		printdebug("problem_type::MOSEK_read - String Parameters");

		sparam = Octave_map();
		get_str_parameters(sparam, task);
	}

	// Initial solution
	if (options.usesol) {
		printdebug("problem_type::MOSEK_read - Initial solution");

		initsol = Octave_map();
		msk_getsolution(initsol, task);
	}

	initialized = true;
}


void problem_type::MOSEK_write(Task_handle &task) {
	if (!initialized) {
		throw msk_exception("Internal error in problem_type::MOSEK_write, no problem was loaded");
	}
	printdebug("Started writing MOSEK problem input");

	/* Set problem description */
	msk_loadproblem(task, sense, c, c0,
			A, blc, buc, blx, bux,
			cones, intsub);

	/* Set initial solution */
	if (options.usesol) {
		append_initsol(task, initsol, numcon, numvar);
	}

	/* Set parameters */
	if (options.useparam) {
		append_parameters(task, iparam, dparam, sparam);
	}

	printdebug("MOSEK_write finished");
}
