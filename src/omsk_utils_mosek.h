#ifndef OMSK_UTILS_MOSEK_H_
#define OMSK_UTILS_MOSEK_H_

#include "omsk_msg_mosek.h"
#include "omsk_obj_mosek.h"

#include "omsk_obj_arguments.h"
#include "omsk_obj_constraints.h"

#include <string>


// ------------------------------
// MOSEK-UTILS
// ------------------------------

// Convert objective sense to and from MOSEK and R
std::string get_objective(MSKobjsensee sense);
MSKobjsensee get_mskobjective(std::string sense);

// Enable fast typing without prefixes
void append_mskprefix(std::string &str, std::string prefix);
void remove_mskprefix(std::string &str, std::string prefix);

// Gets and sets the constraint and variable bounds in task
void set_boundkey(double bl, double bu, MSKboundkeye *bk);
void get_boundvalues(MSKtask_t task, double *lower, double* upper, MSKaccmodee boundtype, MSKintt numbounds);

// Gets and sets the parameters in task
void set_parameter(MSKtask_t task, std::string type, std::string name, octave_value value);
void append_parameters(MSKtask_t task, Octave_map& iparam, Octave_map& dparam, Octave_map& sparam);
void get_int_parameters(Octave_map &paramvec, MSKtask_t task);
void get_dou_parameters(Octave_map &paramvec, MSKtask_t task);
void get_str_parameters(Octave_map &paramvec, MSKtask_t task);

// Get and set solutions in task
void msk_getsolution(Octave_map &solvec, MSKtask_t task);
void append_initsol(MSKtask_t task, Octave_map initsol, int NUMCON, int NUMVAR);

// Initialise the task and load problem from arguments
void msk_loadproblem(Task_handle &task,
					   MSKobjsensee sense, RowVector cvec, double c0,
					   SparseMatrix &A,
					   RowVector blcvec, RowVector bucvec,
					   RowVector blxvec, RowVector buxvec,
					   conicSOC_type &cones, int32NDArray &intsubvec);


#endif /* OMSK_UTILS_MOSEK_H_ */
