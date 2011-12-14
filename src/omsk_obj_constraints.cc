#include "omsk_obj_constraints.h"

#include "omsk_utils_octave.h"
#include "omsk_utils_mosek.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// ------------------------------
// Class conicSOC_type
// ------------------------------

const conicSOC_type::ITEMS_type::OCT_ARGS_type conicSOC_type::ITEMS_type::OCT_ARGS;


void conicSOC_type::OCT_read(Cell &object) {
	if (initialized) {
		throw msk_exception("Internal error in conicSOC_type::OCT_read, a SOC list was already loaded");
	}

	printdebug("Started reading second order cone list from Octave");
	cones = object;
	numcones = cones.nelem();

	initialized = true;
}


void conicSOC_type::OCT_write(Cell &val) {
	if (!initialized) {
		throw msk_exception("Internal error in conicSOC_type::OCT_write, no SOC list loaded");
	}

	printdebug("Started writing second order cone list to Octave");
	val = cones;
}


void conicSOC_type::MOSEK_read(Task_handle &task) {
	if (initialized) {
		throw msk_exception("Internal error in conicSOC_type::MOSEK_read, a SOC list was already loaded");
	}

	printdebug("Started reading second order cone list from MOSEK");

	errcatch( MSK_getnumcone(task, &numcones) );
	cones = Cell(Array<octave_value>(dim_vector(1,numcones)));
	octave_value *conesvec = cones.fortran_vec();

	for (int i=0; i<numcones; i++) {
		Octave_map cone;

		MSKintt numconemembers;
		MSK_getnumconemem(task, i, &numconemembers);

		// TODO: Can this inefficient element-by-element copy construction be avoided?
		// Array<int32_t> have int32_t* array access from fortran_vec, but can not be returned!
		//   - No constructor octave_value(&Array<int32_t>)
		//   - No conversion to Array<octave_int32_t>
		// Array<octave_int32_t> can be returned, but have no int32_t* array access!
		//   - Conversion to Array<int32_t> is just a copy

		MSKidxt submem[numconemembers];
		int32NDArray subvec(dim_vector(1,numconemembers));
		octave_int32 *psub = subvec.fortran_vec();

		MSKconetypee msktype;
		errcatch( MSK_getcone(task, i, &msktype, NULL, &numconemembers, submem) );

		// Octave indexes count from 1, not from 0 as MOSEK
		for (int k=0; k<numconemembers; k++) {
			psub[k] = octave_int32(submem[k] + 1);
		}

		char type[MSK_MAX_STR_LEN];
		errcatch( MSK_conetypetostr(task, msktype, type) );

		string strtype = type;
		remove_mskprefix(strtype,"MSK_CT_");

		cone.assign("type", octave_value(strtype, '\"'));
		cone.assign("sub", octave_value(subvec));

		conesvec[i] = octave_value(cone);
	}
	initialized = true;
}


void conicSOC_type::MOSEK_write(Task_handle &task) {
	if (!initialized) {
		throw msk_exception("Internal error in conicSOC_type::MOSEK_write, no SOC list loaded");
	}

	printdebug("Started writing second order cone list to MOSEK");

	for (MSKidxt idx=0; idx<numcones; ++idx) {

		Octave_map cone = cones.elem(idx).map_value();
		if (error_state)
			throw msk_exception("The cone at index " + tostring(idx+1) + " should be a 'struct'");

		string type;		map_seek_String(&type, cone, ITEMS.OCT_ARGS.type);
		int32NDArray sub;	map_seek_IntegerArray(&sub, cone, ITEMS.OCT_ARGS.sub);
		validate_OctaveMap(cone, "cones{" + tostring(idx+1) + "}", ITEMS.OCT_ARGS.arglist);

		// Convert type to mosek input
		strtoupper(type);
		append_mskprefix(type, "MSK_CT_");
		char msktypestr[MSK_MAX_STR_LEN];
		if(!MSK_symnamtovalue(const_cast<MSKCONST char*>(type.c_str()), msktypestr))
			throw msk_exception("The type of cone at index " + tostring(idx+1) + " was not recognized");

		MSKconetypee msktype = (MSKconetypee)atoi(msktypestr);

		// Convert sub type and indexing (Minus one because MOSEK indexes counts from 0, not from 1 as Octave)
		int msksub[sub.nelem()];
		octave_int32 *psub = sub.fortran_vec();
		for (int i=0; i < sub.nelem(); i++)
			msksub[i] = psub[i].value() - 1;

		// Append the cone
		errcatch( MSK_appendcone(task,
				msktype,		/* The type of cone */
				0.0, 			/* For future use only, can be set to 0.0 */
				sub.nelem(),	/* Number of variables */
				msksub) );		/* Variable indexes */
	}
}
