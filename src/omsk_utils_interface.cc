#include "omsk_utils_interface.h"

#include "omsk_utils_mosek.h"

#include <string>
#include <exception>

using std::string;
using std::exception;


// ------------------------------
// Cleaning and termination code
// ------------------------------

void reset_global_ressources() {
	// The mosek environment 'global_env' should not be cleared, as we wish to
	// reuse the license until mosek_clean() is called or .SO/.DLL is unloaded.

	delete_all_pendingmsg();
}

void reset_global_variables() {
	mosek_interface_verbose  = NAN;   	// Declare messages as pending
	mosek_interface_warnings = 0;

	set_global_value("Rmosek", empty_octave_value);
}

void terminate_successfully(Octave_map &ret_val) /* nothrow */ {
	try {
		msk_addresponse(ret_val, get_msk_response(MSK_RES_OK), false);
		reset_global_ressources();
	}
	catch (exception const& e) { /* Just terminate.. */ }

	// The Octave API force us to quit without return value if signals have been caught.
	// See the TODO on 'octave_signal_caught'.
	try {
		OCTAVE_QUIT;
	} catch (octave_interrupt_exception const& e) {
		set_global_value("Rmosek", ret_val);
		printinfo("Results saved to global variable 'Rmosek'");
		throw; // this is important!!
	}
}

void terminate_unsuccessfully(Octave_map &ret_val, const msk_exception &e) /* nothrow */ {
	try {
		// Force pending and future messages through
		if (xisnan(mosek_interface_verbose)) {
			mosek_interface_verbose = typeALL;
			printoutput("----- PENDING MESSAGES -----\n", typeERROR);
		}
		printpendingmsg();
		printerror( e.what() );

		msk_addresponse(ret_val, e.getresponse(), true);
	}
	catch (exception const& e) { /* Just terminate.. */ }
	terminate_successfully(ret_val);
}

void terminate_unsuccessfully(Octave_map &ret_val, const char* msg) /* nothrow */ {
	try {
		terminate_unsuccessfully(ret_val, msk_exception(msk_response(string(msg))));
	}
	catch (exception const& e) { /* Just terminate.. */ }
}

void msk_addresponse(Octave_map &ret_val, const msk_response &res, bool overwrite) {

	// Using overwrite=false, the MSK_RES_OK can be added only if no other response exists.
	// Using overwrite=true, errors which calls for immediate exit can overwrite the regular response.
	if (ret_val.contains("response") && !overwrite)
		return;

	Octave_map res_vec;
	res_vec.assign("code", octave_value(res.code));
	res_vec.assign("msg", octave_value(res.msg, '\"'));

	ret_val.assign("response", octave_value(res_vec));
}


// ------------------------------
// Main interface functionality
// ------------------------------


// Interrupts MOSEK if CTRL+C is caught in Octave
static int MSKAPI mskcallback(MSKtask_t task, MSKuserhandle_t handle, MSKcallbackcodee caller) {

	if (octave_signal_caught) {
		printoutput("Interruption caught, terminating at first chance...\n", typeERROR);
		return 1;
	}
	return 0;
}


/* Solve a loaded problem and return the solution */
void msk_solve(Octave_map &ret_val, Task_handle &task, options_type options) {


	printdebug("msk_solve - INITIALIZATION");
	{
		/* Make it interruptible with CTRL+C */
		errcatch( MSK_putcallbackfunc(task, mskcallback, (void*)NULL) );

		/* Write file containing problem description (filetypes: .lp, .mps, .opf, .mbt) */
		if (!options.writebefore.empty()) {
			MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_ON);
			errcatch( MSK_writedata(task, const_cast<MSKCONST char*>(options.writebefore.c_str())) );
		}
	}



	printdebug("msk_solve - OPTIMIZATION");
	try
	{
		/* Separate interface warnings from MOSEK output */
		if (mosek_interface_warnings > 0)
			printoutput("\n", typeWARNING);

		/* Run optimizer */
		MSKrescodee trmcode;
		errcatch( MSK_optimizetrm(task, &trmcode) );
		msk_addresponse(ret_val, get_msk_response(trmcode));

	} catch (exception const& e) {
		// Report that the CTRL+C interruption has been caught
		if (octave_signal_caught) {

//			TODO: FIND A WAY TO ACHIEVE SOMETHING LIKE THIS:
//
//			octave_signal_caught = 0;    // <-- Tell Octave that the CTRL+C interruption has been handled.
//										 //		This will allow return values, but will create problems if
//										 //     the variable 'octave_interrupt_state' is not reset.
//			octave_interrupt_state = 0;  // <-- this line currently does not work, and their are currently
//										 //     no other ways to reset it. If it accumulates to 3, you get
//										 //     panic: Interrupt -- stopping myself...

			printoutput("Optimization interrupted because of termination signal, e.g. <CTRL> + <C>.\n", typeERROR);

		} else {
			printoutput("Optimization interrupted.\n", typeERROR);
		}
		throw;
	}



	printdebug("msk_solve - EXTRACT SOLUTION");
	try
	{
		/* Write file containing problem description (solution only included if filetype is .opf or .mbt) */
		if (!options.writeafter.empty()) {
			MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_ON);
			errcatch( MSK_writedata(task, const_cast<MSKCONST char*>(options.writeafter.c_str())) );
		}

		/* Print a summary containing information
		 * about the solution for debugging purposes. */
		errcatch( MSK_solutionsummary(task, MSK_STREAM_LOG) );

		/* Extract solution from Mosek to Octave */
		Octave_map sol_val;
		msk_getsolution(sol_val, task);
		ret_val.assign("sol", octave_value(sol_val));

	} catch (exception const& e) {
		printoutput("An error occurred while extracting the solution.\n", typeERROR);
		throw;
	}
}


/* Load a problem description from file */
void msk_loadproblemfile(Task_handle &task, string filepath, options_type &options) {

	// Make sure the environment is initialized
	global_env.init();

	// Initialize the task
	task.init(global_env, 0, 0);

	try {
		errcatch( MSK_readdata(task, const_cast<MSKCONST char*>(filepath.c_str())) );

	} catch (exception const& e) {
		printerror("An error occurred while loading up the problem from a file");
		throw;
	}
}


/* Save a problem description to file */
void msk_saveproblemfile(Task_handle &task, string filepath, options_type &options) {

	// Set export-parameters for whether to write any solution loaded into MOSEK
	if (options.usesol) {
		errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_ON) );
	} else {
		errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_OFF) );
	}

	// Set export-parameters for whether to write all parameters
	if (options.useparam) {
		errcatch( MSK_putintparam(task, MSK_IPAR_WRITE_DATA_PARAM,MSK_ON) );
		errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PARAMETERS,MSK_ON) );
	} else {
		errcatch( MSK_putintparam(task, MSK_IPAR_WRITE_DATA_PARAM,MSK_OFF) );
		errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PARAMETERS,MSK_OFF) );
	}

	// Write to filepath model (filetypes: .lp, .mps, .opf, .mbt)
	errcatch( MSK_writedata(task, const_cast<MSKCONST char*>(filepath.c_str())) );
}
