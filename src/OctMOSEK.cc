// Copyright (C) 2011   MOSEK ApS
// Made by:
//   Henrik Alsing Friberg   <haf@mosek.com>
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Linking this program statically or dynamically with other modules is making 
// a combined work based on this program. Thus, the terms and conditions of 
// the GNU Lesser General Public License cover the whole combination.
//
// In addition, as a special exception, the copyright holders of this program 
// give you permission to combine this program with the MOSEK C/C++ API 
// (or modified versions of such code). You may copy and distribute such a system 
// following the terms of the GNU LGPL for this program and the licenses 
// of the other code concerned.
//
// Note that people who make modified versions of this program are not obligated 
// to grant this special exception for their modified versions; it is their choice 
// whether to do so. The GNU Lesser General Public License gives permission 
// to release a modified version without this exception; this exception also makes 
// it possible to release a modified version which carries forward this exception.
//

#include "omsk_msg_mosek.h"
#include "omsk_utils_interface.h"
#include "omsk_utils_mosek.h"
#include "omsk_obj_arguments.h"
#include "omsk_obj_mosek.h"

#include <octave/oct.h>
#include <octave/ov-struct.h>

#include <string>
#include <exception>

using std::string;
using std::exception;


DEFUN_DLD (__mosek__, args, nargout, "\
r = mosek(problem, opts)                                    \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek__ 								\n\
") {
	const string ARGNAMES[] = {"problem","options"};
	const string ARGTYPES[] = {"struct","struct"};

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_global_variables();
		printdebug("Function 'mosek' was called");

		// Validate input arguments
		Octave_map arg0;
		if (!args.empty()) {
			arg0 = args(0).map_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
			}
		}
		Octave_map arg1;
		if (args.length()-1 >= 1) {
			arg1 = args(1).map_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[1] + " should be a " + ARGTYPES[1] + ".");
			}
		}

		// Read input arguments: problem and options
		problem_type probin;
		probin.options.OCT_read(arg1);
		probin.OCT_read(arg0);

		// Create task and load problem into MOSEK
		Task_handle task;
		probin.MOSEK_write(task);

		// Solve the problem
		msk_solve(ret_val, task, probin.options);

		// Print warning summary
		if (mosek_interface_warnings > 0) {
			printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n\n", typeWARNING);
		}

	} catch (msk_exception const& e) {
 		terminate_unsuccessfully(ret_val, e);
		return octave_value(ret_val);

	} catch (exception const& e) {
		terminate_unsuccessfully(ret_val, e.what());
		return octave_value(ret_val);
	}

	// Clean allocations and exit (msk_solve adds response)
	terminate_successfully(ret_val);
	return octave_value(ret_val);
}


DEFUN_DLD (__mosek_clean__, args, nargout, "\
mosek_clean()                                               \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_clean__                          \n\
") {
	// The mosek_clean function use verbose=typeINFO by default (should we read options?)
	reset_global_variables();
	mosek_interface_verbose = typeINFO;

	// Clean global resources and release the MOSEK environment
	reset_global_ressources();
	global_env.~Env_handle();

	return empty_octave_value;
}


DEFUN_DLD (__mosek_version__, args, nargout, "\
r = mosek_version()                                         \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_version__                        \n\
") {
	MSKintt major, minor, build, revision;
	MSK_getversion(&major, &minor, &build, &revision);

	// Construct output
	string ret_val("MOSEK " + tostring(major) + "." +  tostring(minor) + "." + tostring(build) + "." + tostring(revision));
	return octave_value(ret_val, '\"');
}


DEFUN_DLD (__mosek_read__, args, nargout, "\
r = mosek_read(filepath, opts)                              \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_read__                           \n\
") {
	const string ARGNAMES[] = {"filepath","options"};
	const string ARGTYPES[] = {"string","struct"};

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_global_variables();
		printdebug("Function 'mosek_read' was called");

		// Validate input arguments
		string arg0;
		if (!args.empty()) {
			arg0 = args(0).string_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
			}
		}
		Octave_map arg1;
		if (args.length()-1 >= 1) {
			arg1 = args(1).map_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[1] + " should be a " + ARGTYPES[1] + ".");
			}
		}

		// Define new default values for options
		options_type default_opts; {
			default_opts.useparam = false;
			default_opts.usesol = false;
		}

		// Read input arguments: options (with modified defaults)
		problem_type probin;
		probin.options = default_opts;
		probin.options.OCT_read(arg1);

		// Create task and load filepath-model into MOSEK
		Task_handle task;
		msk_loadproblemfile(task, arg0, probin.options);

		// Read the problem from MOSEK
		probin.MOSEK_read(task);

		// Write the problem to Octave
		Octave_map prob_val;
		probin.OCT_write(prob_val);
		ret_val.assign("prob", octave_value(prob_val));

		// Add the response code
		msk_addresponse(ret_val, get_msk_response(MSK_RES_OK), false);

		// Print warning summary
		if (mosek_interface_warnings > 0) {
			printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n", typeWARNING);
		}

	} catch (msk_exception const& e) {
		terminate_unsuccessfully(ret_val, e);
		return octave_value(ret_val);

	} catch (exception const& e) {
		terminate_unsuccessfully(ret_val, e.what());
		return octave_value(ret_val);
	}

	// Clean allocations, add response and exit
	terminate_successfully(ret_val);
	return octave_value(ret_val);
}


DEFUN_DLD (__mosek_write__, args, nargout, "\
r = mosek_write(problem, filepath, opts)                    \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_write__                          \n\
") {
	const string ARGNAMES[] = {"problem","filepath","options"};
	const string ARGTYPES[] = {"struct","string","struct"};

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_global_variables();
		printdebug("Function 'mosek_write' was called");

		// Validate input arguments
		Octave_map arg0;
		if (!args.empty()) {
			arg0 = args(0).map_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
			}
		}
		string arg1;
		if (args.length()-1 >= 1) {
			arg1 = args(1).string_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[1] + " should be a " + ARGTYPES[1] + ".");
			}
		}
		Octave_map arg2;
		if (args.length()-1 >= 2) {
			arg2 = args(2).map_value();
			if (error_state) {
				throw msk_exception("Input argument " + ARGNAMES[2] + " should be a " + ARGTYPES[2] + ".");
			}
		}

		// Define new default values for options
		options_type default_opts; {
			default_opts.useparam = false;
			default_opts.usesol = false;
		}

		// Read input arguments: options (with modified defaults)
		problem_type probin;
		probin.options = default_opts;
		probin.options.OCT_read(arg2);
		probin.OCT_read(arg0);

		// Create task and load problem into MOSEK
		Task_handle task;
		probin.MOSEK_write(task);

		// Write the loaded problem to a file
		msk_saveproblemfile(task, arg1, probin.options);

		// Add the response code
		msk_addresponse(ret_val, get_msk_response(MSK_RES_OK), false);

		// Print warning summary
		if (mosek_interface_warnings > 0) {
			printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n", typeWARNING);
		}

	} catch (msk_exception const& e) {
		terminate_unsuccessfully(ret_val, e);
		return octave_value(ret_val);

	} catch (exception const& e) {
		terminate_unsuccessfully(ret_val, e.what());
		return octave_value(ret_val);
	}

	// Clean allocations, add response and exit
	terminate_successfully(ret_val);
	return octave_value(ret_val);
}
