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
	~Env_handle();

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
	~Task_handle();
};

#endif /* OMSK_OBJ_MOSEK_H_ */
