#ifndef OMSK_UTILS_INTERFACE_H_
#define OMSK_UTILS_INTERFACE_H_

#include "omsk_msg_mosek.h"
#include "omsk_obj_mosek.h"
#include "omsk_obj_arguments.h"


// ------------------------------
// Cleaning and termination code
// ------------------------------
void reset_global_ressources();
void reset_global_variables();
void terminate_successfully(Octave_map &ret_val);
void terminate_unsuccessfully(Octave_map &ret_val, const msk_exception &e);
void terminate_unsuccessfully(Octave_map &ret_val, const char* msg);
void msk_addresponse(Octave_map &ret_val, const msk_response &res, bool overwrite=true);


// ------------------------------
// Main interface functionality
// ------------------------------

// Solve a loaded problem and return the solution
void msk_solve(Octave_map &ret_val, Task_handle &task, options_type options);

// Load a problem description from file
void msk_loadproblemfile(Task_handle &task, std::string filepath, options_type &options);

// Save a problem description to file
void msk_saveproblemfile(Task_handle &task, std::string filepath, options_type &options);

#endif /* OMSK_UTILS_INTERFACE_H_ */
