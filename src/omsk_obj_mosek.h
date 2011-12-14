#ifndef OMSK_OBJ_MOSEK_H_
#define OMSK_OBJ_MOSEK_H_

#include "omsk_msg_mosek.h"

// ------------------------------
// Global variable: MOSEK environment
// ------------------------------
extern class Env_handle {
private:
	bool initialized;

	// Overwrite copy constructor and provide no implementation
	Env_handle(const Env_handle& that);

public:
	MSKenv_t env;
	Env_handle() 		{ initialized = false; }
	operator MSKenv_t() { return env; }

	void init();
//		if (!initialized) {
//			printinfo("Acquiring MOSEK environment");
//
//			try {
//				/* Create the mosek environment. */
//				errcatch( MSK_makeenv(&env, NULL, NULL, NULL, NULL) );
//
//				try {
//					/* Directs the env log stream to the 'mskprintstr' function. */
//					errcatch( MSK_linkfunctoenvstream(env, MSK_STREAM_LOG, NULL, mskprintstr) );
//
//					try {
//						/* Initialize the environment. */
//						errcatch( MSK_initenv(env) );
//
//					} catch (exception const& e) {
//						MSK_unlinkfuncfromenvstream(env, MSK_STREAM_LOG);
//						throw;
//					}
//				} catch (exception const& e) {
//					MSK_deleteenv(&env);
//					throw;
//				}
//			} catch (exception const& e) {
//				printerror("Failed to acquire MOSEK environment");
//				throw;
//			}
//
//			initialized = true;
//		}

	~Env_handle();
//		if (initialized) {
//			printinfo("Releasing MOSEK environment");
//			MSK_unlinkfuncfromenvstream(env, MSK_STREAM_LOG);
//			MSK_deleteenv(&env);
//			initialized = false;
//		}

} global_env;


// ------------------------------
// MOSEK Task handle
// ------------------------------
class Task_handle {
private:
	MSKtask_t task;
	bool initialized;

	// Overwrite copy constructor and provide no implementation
	Task_handle(const Task_handle& that);

public:
	Task_handle() 		 { initialized = false; }
	operator MSKtask_t() { return task; }

	void init(MSKenv_t env, MSKintt maxnumcon, MSKintt maxnumvar);
//		if (initialized)
//			throw msk_exception("No support for multiple tasks yet!");
//
//		printdebug("Creating an optimization task");
//
//		/* Create the optimization task. */
//		errcatch( MSK_maketask(env, maxnumcon, maxnumvar, &task) );
//
//		try {
//			/* Directs the log task stream to the 'mskprintstr' function. */
//			errcatch( MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, mskprintstr) );
//
//		} catch (exception const& e) {
//			MSK_deletetask(&task);
//			throw;
//		}
//
//		initialized = true;


	~Task_handle();
//		if (initialized) {
//			printdebug("Removing an optimization task");
//			MSK_unlinkfuncfromtaskstream(task, MSK_STREAM_LOG);
//			MSK_deletetask(&task);
//			initialized = false;
//		}

};

#endif /* OMSK_OBJ_MOSEK_H_ */
