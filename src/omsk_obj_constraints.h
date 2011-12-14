#ifndef OMSK_OBJ_CONSTRAINTS_H_
#define OMSK_OBJ_CONSTRAINTS_H_

#include "omsk_msg_mosek.h"
#include "omsk_obj_mosek.h"

#include <string>
#include <vector>

class conicSOC_type {
private:
	bool initialized;

public:
	// Recognised second order cone arguments in R
	// TODO: Upgrade to new C++11 initialisers
	struct ITEMS_type {
		static const struct OCT_ARGS_type {

			std::vector<std::string> arglist;
			const std::string type;
			const std::string sub;

			OCT_ARGS_type() :
				type("type"),
				sub("sub")
			{
				std::string temp[] = {type, sub};
				arglist = std::vector<std::string>(temp, temp + sizeof(temp)/sizeof(std::string));
			}
		} OCT_ARGS;
	} ITEMS;


	// Data definition (intentionally kept close to R types)
	MSKintt numcones;
	Cell cones;

	// Simple construction and destruction
	conicSOC_type() : initialized(false) {}
	~conicSOC_type() {}

	// Read and write matrix from and to Octave
	void OCT_read(Cell &object);
	void OCT_write(Cell &val);

	// Read and write matrix from and to MOSEK
	void MOSEK_read(Task_handle &task);
	void MOSEK_write(Task_handle &task);
};


#endif /* OMSK_OBJ_CONSTRAINTS_H_ */
