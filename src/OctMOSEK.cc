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

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include "mosek.h"

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <octave/Matrix.h>

using namespace std;

// Global initialization
double mosek_interface_verbose  = NAN;				// Declare messages as pending
int    mosek_interface_warnings = 0;				// Start warning count from zero

// ------------------------------
// RESPONSE AND EXCEPTION SYSTEM
// ------------------------------

string msk_responsecode2text(MSKrescodee r) {
	char symname[MSK_MAX_STR_LEN];
	char desc[MSK_MAX_STR_LEN];

	if (MSK_getcodedesc(r, symname, desc) == MSK_RES_OK) {
		return string(symname) + ": " + string(desc) + "";
	} else {
		return "The response code could not be identified";
	}
}

struct msk_response {
public:
	double code;
	string msg;

	msk_response(MSKrescodee r) {
		code = (double)r;
		msg = msk_responsecode2text(r);
	}

	msk_response(const string &msg) :
		code(NAN), msg(msg) {}

	msk_response(double code, const string &msg) :
		code(code), msg(msg) {}
};

struct msk_exception : public runtime_error {
public:
	const double code;

	// Used for MOSEK errors with response codes
	msk_exception(const msk_response &res) : runtime_error(res.msg), code(res.code)
	{}

	// Used for interface errors without response codes
	msk_exception(const string &msg) : runtime_error(msg), code(NAN)
	{}

	msk_response getresponse() const {
		return msk_response(code, what());
	}
};

// ------------------------------
// PRINTING SYSTEM
// ------------------------------
enum verbosetype { typeERROR=1, typeMOSEK=2, typeWARNING=3, typeINFO=4, typeDEBUG=50, typeALL=100 };
vector<string>      mosek_pendingmsg_str;
vector<verbosetype> mosek_pendingmsg_type;

void printoutput(string str, verbosetype strtype) {
	if (isnan(mosek_interface_verbose)) {
		mosek_pendingmsg_str.push_back(str);
		mosek_pendingmsg_type.push_back(strtype);

	} else if (mosek_interface_verbose >= strtype) {
		octave_stdout << str;

		// This does not work well with a PAGER..
		// flush_octave_stdout();
	}
}

void printerror(string str) {
	printoutput("ERROR: " + str + "\n", typeERROR);
}

void printwarning(string str) {
	printoutput("WARNING: " + str + "\n", typeWARNING);
	mosek_interface_warnings++;
}

void printinfo(string str) {
	printoutput(str + "\n", typeINFO);
}

void printdebug(string str) {
	printoutput(str + "\n", typeDEBUG);
}

void printdebugdata(string str) {
	printoutput(str, typeDEBUG);
}

void printpendingmsg() {
	if (mosek_pendingmsg_str.size() != mosek_pendingmsg_type.size())
		throw msk_exception("Error in handling of pending messages");

	for (size_t i=0; i<mosek_pendingmsg_str.size(); i++) {
		printoutput(mosek_pendingmsg_str[i], mosek_pendingmsg_type[i]);
	}

	mosek_pendingmsg_str.clear();
	mosek_pendingmsg_type.clear();
}

// This function translates MOSEK response codes into msk_exceptions.
void errcatch(MSKrescodee r, string str) {
	if (r != MSK_RES_OK) {
		if (!str.empty())
			printerror(str);

		throw msk_exception(r);
	}
}
void errcatch(MSKrescodee r) {
	errcatch(r, "");
}

// ------------------------------
// OCTAVE HELPERS
// ------------------------------

octave_value empty_octave_value = octave_value(octave_value_list());

template <class T>
string tostring(T val)
{
	ostringstream ss;
	ss << val;
	return ss.str();
}

void strtoupper(string &str) {
	string::iterator i = str.begin();
	string::iterator end = str.end();

	while (i != end) {
	    *i = toupper((int)*i);
	    ++i;
	}
}

/* This function returns the sign of doubles even when incomparable (isinf true). */
bool ispos(double x) {
	return (copysign(1.0, x) > 0);
}

/* This function converts from the normal double type to integers (supports infinity) */
int scalar2int(double scalar) {
	if (isinf(scalar))
		return INT_MAX;

	int retval = (int)scalar;
	double err = scalar - retval;

	if (err <= -1e-6 || 1e-6 <= err)
		printwarning("A scalar with fractional value was truncated to an integer.");

	return retval;
}

bool isEmpty(octave_value& obj) {
	if (obj.is_empty())
		return true;

	// Also count NA and NaN as empty
	if (obj.is_scalar_type()) {
		if (isnan(obj.scalar_value()))
			if (!error_state)
				return true;
	}
	error_state = false;

	return false;
}
bool isEmpty(Octave_map& obj) {
	return (obj.nfields() == 0);
}
template<class T>
bool isEmpty(Array<T>& obj) {
	return (obj.nelem() == 0);
}
bool isEmpty(SparseMatrix& obj) {
	return (obj.nnz() == 0);
}

/* This function prints the output log from MOSEK to the terminal. */
static void MSKAPI mskprintstr(void *handle, char str[]) {
	printoutput(str, typeMOSEK);
}

// ------------------------------
// HANDLES FOR PROTECTED RESOURCES
// ------------------------------

/* This class handles the MOSEK environment resource */
class Env_handle {
private:
	bool initialized;

	// Overwrite copy constructor and provide no implementation
	Env_handle(const Env_handle& that);

public:
	MSKenv_t env;
	Env_handle() 		{ initialized = false; }
	operator MSKenv_t() { return env; }

	void init() {
		if (!initialized) {
			printinfo("Acquiring MOSEK environment");

			try {
				/* Create the mosek environment. */
				errcatch( MSK_makeenv(&env, NULL, NULL, NULL, NULL) );

				try {
					/* Directs the env log stream to the 'mskprintstr' function. */
					errcatch( MSK_linkfunctoenvstream(env, MSK_STREAM_LOG, NULL, mskprintstr) );

					try {
						/* Initialize the environment. */
						errcatch( MSK_initenv(env) );

					} catch (exception const& e) {
						MSK_unlinkfuncfromenvstream(env, MSK_STREAM_LOG);
						throw;
					}
				} catch (exception const& e) {
					MSK_deleteenv(&env);
					throw;
				}
			} catch (exception const& e) {
				printerror("Failed to acquire MOSEK environment");
				throw;
			}

			initialized = true;
		}
	}

	~Env_handle() {
		if (initialized) {
			printinfo("Releasing MOSEK environment");
			MSK_unlinkfuncfromenvstream(env, MSK_STREAM_LOG);
			MSK_deleteenv(&env);
			initialized = false;
		}
	}
} global_env;

/* This class handles the MOSEK task resource */
class Task_handle {
private:
	MSKtask_t task;
	bool initialized;

	// Overwrite copy constructor and provide no implementation
	Task_handle(const Task_handle& that);

public:
	Task_handle() 		 { initialized = false; }
	operator MSKtask_t() { return task; }

	void init(MSKenv_t env, MSKintt maxnumcon, MSKintt maxnumvar) {
		if (initialized)
			throw msk_exception("No support for multiple tasks yet!");

		printdebug("Creating an optimization task");

		/* Create the optimization task. */
		errcatch( MSK_maketask(env, maxnumcon, maxnumvar, &task) );

		try {
			/* Directs the log task stream to the 'mskprintstr' function. */
			errcatch( MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, mskprintstr) );

		} catch (exception const& e) {
			MSK_deletetask(&task);
			throw;
		}

		initialized = true;
	}

	~Task_handle() {
		if (initialized) {
			printdebug("Removing an optimization task");
			MSK_unlinkfuncfromtaskstream(task, MSK_STREAM_LOG);
			MSK_deletetask(&task);
			initialized = false;
		}
	}
};

// ------------------------------
// OCTAVE HELPERS
// ------------------------------

//
// Handling of: octave_value
//
void map_seek_Value(octave_value *out, Octave_map map, string name, bool optional=false)
{
	Octave_map::const_iterator iter = map.seek(name);
	if (iter != map.end())
		*out = map.contents(iter)(0);
	else if (!optional)
		throw msk_exception("An expected variable named '" + name + "' was not found");
}

//
// Handling of: Octave_map
//
void map_seek_OctaveMap(Octave_map *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable '" + name + "' needs a non-empty definition");;
	}

	Octave_map temp = val.map_value();
	if (error_state)
		throw msk_exception("Variable \"" + name + "\" should be a 'struct'");

	*out = temp;
}
void validate_OctaveMap(Octave_map& object, string name, vector<string> keywords, bool optional=false)
{
	if (optional && isEmpty(object))
		return;

	for (Octave_map::iterator p = object.begin(); p != object.end(); p++) {
		string key = object.key(p);
		bool recognized = false;

		for (size_t j = 0; j < keywords.size() && !recognized; j++)
			if (key == keywords[j])
				recognized = true;

		if (!recognized) {
			if (name == "")
				throw msk_exception("Variable \"" + key + "\" in structure not recognized");
			else
				throw msk_exception("Variable \"" + key + "\" in structure \"" + name + "\" not recognized");
		}
	}
}

//
// Handling of: Cell
//
void map_seek_Cell(Cell *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	Cell temp = val.cell_value();
	if (error_state)
		throw msk_exception("Variable \"" + name + "\" should be a Cell");

	*out = temp;
}
void validate_Cell(Cell &object, string name, int nelem, bool optional=false)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nelem)
		throw msk_exception("Cell \"" + name + "\" has the wrong dimensions");
}

//
// Handling of: SparseMatrix
//
void map_seek_SparseMatrix(SparseMatrix *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	SparseMatrix temp = val.sparse_matrix_value();
	if (error_state)
		throw msk_exception("Variable \"" + name + "\" should be a Sparse Matrix");

	*out = temp;
}
void validate_SparseMatrix(SparseMatrix& object, string name, int nrows, int ncols, bool optional=false)
{
	if (optional && isEmpty(object))
		return;

	if (object.dim1() != nrows || object.dim2() != ncols)
		throw msk_exception("Sparse Matrix \"" + name + "\" has the wrong dimensions");
}

//
// Handling of: Row Vector (use IntegerArray for integer-typed vectors)
//
void map_seek_RowVector(RowVector *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	RowVector temp = val.row_vector_value();
	if (error_state)
		throw msk_exception("Variable \"" + name + "\" should be a Vector");

	*out = temp;
}
void validate_RowVector(RowVector& object, string name, int nrows, bool optional=false)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nrows)
		throw msk_exception("Vector \"" + name + "\" has the wrong dimensions");
}

//
// Handling of: IntegerArray
//
void map_seek_IntegerArray(int32NDArray *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	int32NDArray temp = val.int32_array_value();
	if (error_state)
		throw msk_exception("Variable \"" + name + "\" should be a Vector with integer-typed entries");

	*out = temp;
}
void validate_IntegerArray(int32NDArray& object, string name, int nrows, bool optional=false)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nrows)
		throw msk_exception("Integer Vector \"" + name + "\" has the wrong dimensions");
}

//
// Handling of: Scalar
//
void map_seek_Scalar(double *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	RowVector temp = val.row_vector_value();
	if (error_state || temp.nelem() != 1)
		throw "Variable \"" + name + "\" should be a Scalar";

	*out = temp.elem(0);
}

//
// Handling of: String
//
void map_seek_String(string *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	string temp = val.string_value();
	if (error_state)
		throw "Variable \"" + name + "\" should be a String";

	*out = temp;
}

//
// Handling of: BOOLEAN
//
void map_seek_Boolean(bool *out, Octave_map& map, string name, bool optional=false)
{
	octave_value val = empty_octave_value;
	map_seek_Value(&val, map, name, optional);

	if (isEmpty(val)) {
		if (optional)
			return;
		else
			throw msk_exception("Variable \"" + name + "\" needs a non-empty definition");
	}

	bool temp = val.bool_value();
	if (error_state)
		throw msk_exception("Argument '" + name + "' should be a Boolean");

	*out = temp;
}

// ------------------------------
// MOSEK-UTILS
// ------------------------------

/* This function explains the solution types of MOSEK. */
string getspecs_sense(MSKobjsensee sense)
{
	switch (sense) {
		case MSK_OBJECTIVE_SENSE_MAXIMIZE:
			return "maximize";
		case MSK_OBJECTIVE_SENSE_MINIMIZE:
			return "minimize";
		case MSK_OBJECTIVE_SENSE_UNDEFINED:
			return "UNDEFINED";
		default:
			throw msk_exception("A problem sense was not supported");
	}
}

/* This function interprets string "sense"  */
MSKobjsensee get_objective(string sense)
{
	if (sense == "min" || sense == "minimize") {
		return MSK_OBJECTIVE_SENSE_MINIMIZE;

	} else if (sense == "max" || sense == "maximize") {
		return MSK_OBJECTIVE_SENSE_MAXIMIZE;

	} else {
		throw msk_exception("Variable 'sense' must be either 'min', 'minimize', 'max' or 'maximize'");
	}
}

/* Enable fast typing without prefixes */
void append_mskprefix(string &str, string prefix)
{
	size_t inclusion = 0, j = 0;
	for (size_t i = 0; i < prefix.length(); i++) {
		if (prefix[i] == str[j]) {
			if (++j >= str.length()) {
				inclusion = prefix.length();
				break;
			}
		} else {
			j = 0;
			inclusion = i+1;
		}
	}
	str = prefix.substr(0, inclusion) + str;
}

/* Enable fast typing without prefixes */
void remove_mskprefix(string &str, string prefix)
{
	if (str.size() > prefix.size()) {
		if (str.substr(0, prefix.size()) == prefix) {
			str = str.substr(prefix.size(), str.size() - prefix.size());
		}
	}
}

/* This function appends a conic constraint to the MOSEK task */
void append_cone(MSKtask_t task, octave_value conevalue, int idx)
{
	// Get input map
	Octave_map cone = conevalue.map_value();
	if (error_state)
		throw msk_exception("The cone at index " + tostring(idx+1) + " should be a 'struct'");

	const struct OCT_CONES_type {
		vector<string> arglist;
		const string type;
		const string sub;

		OCT_CONES_type() :
			type("type"),
			sub("sub")
		{
			string temp[] = {type, sub};
			arglist = vector<string>(temp, temp + sizeof(temp)/sizeof(string));
		}
	} OCT_CONES = OCT_CONES_type();

	string type;		map_seek_String(&type, cone, OCT_CONES.type);
	int32NDArray sub;	map_seek_IntegerArray(&sub, cone, OCT_CONES.sub);
	validate_OctaveMap(cone, "cones{" + tostring(idx+1) + "}", OCT_CONES.arglist);

	// Convert type to MOSEK input
	strtoupper(type);
	append_mskprefix(type, "MSK_CT_");
	char msktypestr[MSK_MAX_STR_LEN];
	if(!MSK_symnamtovalue(const_cast<MSKCONST char*>(type.c_str()), msktypestr))
		throw msk_exception("The type of cone at index " + tostring(idx+1) + " was not recognized");

	MSKconetypee msktype = (MSKconetypee)atoi(msktypestr);

	// Convert sub type and indexing (Minus one because MOSEK indexes counts from 0, not from 1 as Octave)
	int msksub[sub.nelem()];
	octave_int32 *psub = sub.fortran_vec();
	for (int i=0; i<sub.nelem(); i++)
		msksub[i] = psub[i].value() - 1;

	// Append the cone
	errcatch( MSK_appendcone(task,
			msktype,		/* The type of cone */
			0.0, 			/* For future use only, can be set to 0.0 */
			sub.nelem(),	/* Number of variables */
			msksub) );		/* Variable indexes */
}

void append_initsol(MSKtask_t task, Octave_map initsol, int NUMCON, int NUMVAR)
{
	for (Octave_map::iterator solitr = initsol.begin();
							  solitr != initsol.end();
							  solitr++)
	{
		string name = initsol.key(solitr);
		printdebug("Reading the initial solution '" + name + "'");

		// Get current solution type
		MSKsoltypee cursoltype;
		if (name == "bas") {
			cursoltype = MSK_SOL_BAS;
		} else if (name == "itr") {
			cursoltype = MSK_SOL_ITR;
		} else if (name == "int") {
			cursoltype = MSK_SOL_ITG;
		} else {
			printwarning("The initial solution '" + name + "' was not recognized.");
			continue;
		}

		// Get current solution
		octave_value val = initsol.contents(solitr)(0);
		if (isEmpty(val)) {
			printwarning("The initial solution \"" + name + "\" was ignored.");
			continue;
		}
		Octave_map cursol = val.map_value();
		if (error_state) {
			throw msk_exception("The initial solution \"" + name + "\" should be a 'struct'");
		}

		// Get current solution items
		Cell skc; 		map_seek_Cell(&skc, cursol, "skc", true);		validate_Cell(skc, "skc", NUMCON, true);
		RowVector xc;  	map_seek_RowVector(&xc, cursol, "xc", true);	validate_RowVector(xc, "xc", NUMCON, true);
		RowVector slc; 	map_seek_RowVector(&slc, cursol, "slc", true);	validate_RowVector(slc, "slc", NUMCON, true);
		RowVector suc; 	map_seek_RowVector(&suc, cursol, "suc", true);	validate_RowVector(suc, "suc", NUMCON, true);

		Cell skx; 		map_seek_Cell(&skx, cursol, "skx", true);		validate_Cell(skx, "skx", NUMVAR, true);
		RowVector xx;  	map_seek_RowVector(&xx, cursol, "xx", true);	validate_RowVector(xx, "xx", NUMVAR, true);
		RowVector slx; 	map_seek_RowVector(&slx, cursol, "slx", true);	validate_RowVector(slx, "slx", NUMVAR, true);
		RowVector sux; 	map_seek_RowVector(&sux, cursol, "sux", true);	validate_RowVector(sux, "sux", NUMVAR, true);
		RowVector snx; 	map_seek_RowVector(&snx, cursol, "snx", true);	validate_RowVector(snx, "snx", NUMVAR, true);

		bool anyinfocon = !isEmpty(skc) || !isEmpty(xc) || !isEmpty(slc) || !isEmpty(suc);
		bool anyinfovar = !isEmpty(skx) || !isEmpty(xx) || !isEmpty(slx) || !isEmpty(sux) || !isEmpty(snx);

		// Set all constraints
		if (anyinfocon) {
			for (int ci = 0; ci < NUMCON; ci++)
			{
				MSKstakeye curskc;
				if (!isEmpty(skc) || !isEmpty(skc(ci))) {
					curskc = MSK_SK_UNK;
				} else {
					string skcname = skc(ci).string_value();
					if (error_state)
						throw msk_exception("The status keys of constraints in the initial solution \"" + name + "\" should be strings");

					MSKintt skcval;
					errcatch( MSK_strtosk(task, const_cast<MSKCONST char*>(skcname.c_str()), &skcval) );
					curskc = (MSKstakeye)skcval;
				}

				MSKrealt curxc;
				if (isEmpty(xc)) {
					curxc = 0.0;
				} else {
					curxc = xc.elem(ci);
				}

				MSKrealt curslc;
				if (isEmpty(slc)) {
					curslc = 0.0;
				} else {
					curslc = slc.elem(ci);
				}

				MSKrealt cursuc;
				if (isEmpty(suc)) {
					cursuc = 0.0;
				} else {
					cursuc = suc.elem(ci);
				}

				errcatch( MSK_putsolutioni(task,
						MSK_ACC_CON, ci, cursoltype,
						curskc, curxc, curslc, cursuc, 0.0));
			}
		}

		// Set all variables
		if (anyinfovar) {
			for (int xi = 0; xi < NUMVAR; xi++)
			{
				MSKstakeye curskx;
				if (isEmpty(skx) || isEmpty(skx(xi))) {
					curskx = MSK_SK_UNK;
				} else {
					string skxname = skx(xi).string_value();
					if (error_state)
						throw msk_exception("The status keys of variables in the initial solution \"" + name + "\" should be strings");

					MSKintt skxval;
					errcatch( MSK_strtosk(task, const_cast<MSKCONST char*>(skxname.c_str()), &skxval) );
					curskx = (MSKstakeye)skxval;
				}

				MSKrealt curxx;
				if (isEmpty(xx)) {

					// Variable not fully defined => use bound information
					MSKboundkeye bk;
					MSKrealt bl,bu;
					errcatch( MSK_getbound(task,
							MSK_ACC_VAR, xi, &bk, &bl, &bu));

					switch (bk) {
						case MSK_BK_FX:
						case MSK_BK_LO:
						case MSK_BK_RA:
							curxx = bl;
							break;
						case MSK_BK_UP:
							curxx = bu;
							break;
						case MSK_BK_FR:
							curxx = 0.0;
							break;
						default:
							throw msk_exception("Unexpected boundkey when loading initial solution");
					}

				} else {
					curxx = xx.elem(xi);
				}

				MSKrealt curslx;
				if (isEmpty(slx)) {
					curslx = 0.0;
				} else {
					curslx = slx.elem(xi);
				}

				MSKrealt cursux;
				if (isEmpty(sux)) {
					cursux = 0.0;
				} else {
					cursux = sux.elem(xi);
				}

				MSKrealt cursnx;
				if (isEmpty(snx)) {
					cursnx = 0.0;
				} else {
					cursnx = snx.elem(xi);
				}

				errcatch( MSK_putsolutioni(
						task, MSK_ACC_VAR, xi, cursoltype,
						curskx, curxx, curslx, cursux, cursnx));
			}
		}

		if (!anyinfocon && !anyinfovar) {
			printwarning("The initial solution '" + name + "' was ignored.");
		}
	}
}

/* This function checks and sets the bounds of constraints and variables. */
void set_boundkey(double bl, double bu, MSKboundkeye *bk)
{
	if (isnan(bl) || isnan(bu))
		throw msk_exception("NAN values not allowed in bounds");

	if (!isinf(bl) && !isinf(bu) && bl > bu)
		throw msk_exception("The upper bound should be larger than the lower bound");

	if (isinf(bl) && ispos(bl))
		throw msk_exception("+INF values not allowed as lower bound");

	if (isinf(bu) && !ispos(bu))
		throw msk_exception("-INF values not allowed as upper bound");

	// Return the bound key
	if (isinf(bl))
	{
		if (isinf(bu))
			*bk = MSK_BK_FR;
		else
			*bk = MSK_BK_UP;
	}
	else
	{
		if (isinf(bu))
			*bk = MSK_BK_LO;
		else
		{
			// Normally this should be abs(bl-bu) <= eps, but we
			// require bl and bu stems from same double variable.
			if (bl == bu)
				*bk = MSK_BK_FX;
			else
				*bk = MSK_BK_RA;
		}
	}
}

/* This function checks and sets the parameters of the MOSEK task. */
void set_parameter(MSKtask_t task, string type, string name, octave_value value)
{
	if (isEmpty(value)) {
		printwarning("The parameter '" + name + "' from " + type + " was ignored due to an empty definition.");
		return;
	}

	if (!value.is_string() && value.numel() >= 2) {
		throw msk_exception("The parameter '" + name + "' from " + type + " had more than one element in its definition.");
	}

	// Convert name to MOSEK input with correct prefix
	strtoupper(name);
	if (type == "iparam") append_mskprefix(name, "MSK_IPAR_"); else
	if (type == "dparam") append_mskprefix(name, "MSK_DPAR_"); else
	if (type == "sparam") append_mskprefix(name, "MSK_SPAR_"); else
		throw msk_exception("A parameter type was not recognized");

	MSKparametertypee ptype;
	MSKintt pidx;
	errcatch( MSK_whichparam(task, const_cast<MSKCONST char*>(name.c_str()), &ptype, &pidx) );

	switch (ptype) {
		case MSK_PAR_INT_TYPE:
		{
			int mskvalue;

			if (value.is_scalar_type())
				mskvalue = scalar2int(value.scalar_value());

			else if (value.is_string()) {
				string valuestr = value.string_value();

				// Convert value string to MOSEK input
				strtoupper(valuestr);
				append_mskprefix(valuestr, "MSK_");

				char mskvaluestr[MSK_MAX_STR_LEN];
				if (!MSK_symnamtovalue(const_cast<MSKCONST char*>(valuestr.c_str()), mskvaluestr))
					throw msk_exception("The value of parameter '" + name + "' from " + type + " was not recognized");

				mskvalue = atoi(mskvaluestr);

			} else {
				throw msk_exception("The value of parameter '" + name + "' from " + type + " should be an integer or string");
			}

			errcatch( MSK_putintparam(task,(MSKiparame)pidx,mskvalue) );
			break;
		}

		case MSK_PAR_DOU_TYPE:
		{
			double mskvalue = value.scalar_value();
			if (error_state)
				throw msk_exception("The value of parameter '" + name + "' from " + type + " should be a double");

			errcatch( MSK_putdouparam(task,(MSKdparame)pidx,mskvalue) );
			break;
		}

		case MSK_PAR_STR_TYPE:
		{
			string mskvalue = value.string_value();
			if (error_state)
				throw msk_exception("The value of parameter " + name + "' from " + type + " should be a string");

			errcatch( MSK_putstrparam(task, (MSKsparame)pidx, const_cast<MSKCONST char*>(mskvalue.c_str())) );
			break;
		}

		default:
			throw msk_exception("Parameter '" + name + "' from " + type + " was not recognized.");
	}
}

void append_parameters(MSKtask_t task, Octave_map& iparam, Octave_map& dparam, Octave_map& sparam) {
	/* Set integer parameters */
	for (Octave_map::iterator p0 = iparam.begin(); p0 != iparam.end(); p0++)
		set_parameter(task, "iparam", iparam.key(p0), iparam.contents(p0)(0));

	/* Set double parameters */
	for (Octave_map::iterator p0 = dparam.begin(); p0 != dparam.end(); p0++)
		set_parameter(task, "dparam", dparam.key(p0), dparam.contents(p0)(0));

	/* Set string parameters */
	for (Octave_map::iterator p0 = sparam.begin(); p0 != sparam.end(); p0++)
		set_parameter(task, "sparam", sparam.key(p0), sparam.contents(p0)(0));
}

/* This function tells if MOSEK define solution item in solution type. */
bool isdef_solitem(MSKsoltypee s, MSKsoliteme v)
{
	switch (v)
	{
		// Primal variables
		case MSK_SOL_ITEM_XC:
		case MSK_SOL_ITEM_XX:
			switch (s) {
			case MSK_SOL_ITR:  return true;
			case MSK_SOL_BAS:  return true;
			case MSK_SOL_ITG:  return true;
			default:
				throw msk_exception("A solution type was not supported");
			}

		// Linear dual variables
		case MSK_SOL_ITEM_SLC:
		case MSK_SOL_ITEM_SLX:
		case MSK_SOL_ITEM_SUC:
		case MSK_SOL_ITEM_SUX:
			switch (s) {
			case MSK_SOL_ITR:  return true;
			case MSK_SOL_BAS:  return true;
			case MSK_SOL_ITG:  return false;
			default:
				throw msk_exception("A solution type was not supported");
			}

		// Conic dual variable
		case MSK_SOL_ITEM_SNX:
			switch (s) {
			case MSK_SOL_ITR:  return true;
			case MSK_SOL_BAS:  return false;
			case MSK_SOL_ITG:  return false;
			default:
				throw msk_exception("A solution type was not supported");
			}

		// Ignored variables
		case MSK_SOL_ITEM_Y:
			return false;

		// Unsupported variables
		default:
			throw msk_exception("A solution item was not supported");
	}
}

/* This function explains the solution types of MOSEK. */
void getspecs_soltype(MSKsoltypee stype, string &name)
{
	switch (stype) {
		case MSK_SOL_BAS:
			name = "bas";
			break;
		case MSK_SOL_ITR:
			name = "itr";
			break;
		case MSK_SOL_ITG:
			name = "int";
			break;
		default:
			throw msk_exception("A solution type was not supported");
	}
}

/* This function explains the solution items of a MOSEK solution type. */
void getspecs_solitem(MSKsoliteme vtype, int NUMVAR, int NUMCON, string &name, int &size)
{
	switch (vtype) {
		case MSK_SOL_ITEM_SLC:
			name = "slc";
			size = NUMCON;
			break;
		case MSK_SOL_ITEM_SLX:
			name = "slx";
			size = NUMVAR;
			break;
		case MSK_SOL_ITEM_SNX:
			name = "snx";
			size = NUMVAR;
			break;
		case MSK_SOL_ITEM_SUC:
			name = "suc";
			size = NUMCON;
			break;
		case MSK_SOL_ITEM_SUX:
			name = "sux";
			size = NUMVAR;
			break;
		case MSK_SOL_ITEM_XC:
			name = "xc";
			size = NUMCON;
			break;
		case MSK_SOL_ITEM_XX:
			name = "xx";
			size = NUMVAR;
			break;
		case MSK_SOL_ITEM_Y:
			name = "y";
			size = NUMCON;
			break;
		default:
			throw msk_exception("A solution item was not supported");
	}
}

/* This function extract the solution from MOSEK. */
void msk_getsolution(Octave_map &solvec, MSKtask_t task)
{
	printdebug("msk_getsolution called");

	MSKintt NUMVAR, NUMCON;
	errcatch( MSK_getnumvar(task, &NUMVAR) );
	errcatch( MSK_getnumcon(task, &NUMCON) );

	// Construct: result -> solution -> solution types
	for (int s=MSK_SOL_BEGIN; s<MSK_SOL_END; ++s)
	{
		Octave_map soltype;

		MSKsoltypee stype = (MSKsoltypee)s;
		MSKintt isdef_soltype;
		errcatch( MSK_solutiondef(task, stype, &isdef_soltype) );

		if (!isdef_soltype)
			continue;

		// Add the problem status and solution status
		MSKprostae prosta;
		MSKsolstae solsta;
		errcatch( MSK_getsolutionstatus(task, stype, &prosta, &solsta) );

		char solsta_str[MSK_MAX_STR_LEN];
		errcatch( MSK_solstatostr(task, solsta, solsta_str) );
		soltype.assign("solsta", octave_value(solsta_str, '\"'));

		char prosta_str[MSK_MAX_STR_LEN];
		errcatch( MSK_prostatostr(task, prosta, prosta_str) );
		soltype.assign("prosta", octave_value(prosta_str, '\"'));

		// Add the constraint status keys
		{
			MSKstakeye *mskskc = new MSKstakeye[NUMCON];
			errcatch( MSK_getsolutionstatuskeyslice(task,
									MSK_ACC_CON,	/* Request constraint status keys. */
									stype,			/* Current solution type. */
									0,				/* Index of first variable. */
									NUMCON,			/* Index of last variable+1. */
									mskskc));

			Cell skcvec(dim_vector(1, NUMCON));
			char skcname[MSK_MAX_STR_LEN];
			for (int ci = 0; ci < NUMCON; ci++) {
				errcatch( MSK_sktostr(task, mskskc[ci], skcname) );
				skcvec.elem(ci) = octave_value(skcname, '\"');
			}
			soltype.assign("skc", octave_value(skcvec));
		}

		// Add the variable status keys
		{
			MSKstakeye *mskskx = new MSKstakeye[NUMVAR];
			errcatch( MSK_getsolutionstatuskeyslice(task,
									MSK_ACC_VAR,	/* Request variable status keys. */
									stype,			/* Current solution type. */
									0,				/* Index of first variable. */
									NUMVAR,			/* Index of last variable+1. */
									mskskx));

			Cell skxvec(dim_vector(1, NUMVAR));
			char skxname[MSK_MAX_STR_LEN];
			for (int xi = 0; xi < NUMVAR; xi++) {
				errcatch( MSK_sktostr(task, mskskx[xi], skxname) );
				skxvec.elem(xi) = octave_value(skxname, '\"');
			}
			soltype.assign("skx", octave_value(skxvec));
		}

		// Add solution variable slices
		for (int v=MSK_SOL_ITEM_BEGIN; v<MSK_SOL_ITEM_END; ++v)
		{
			MSKsoliteme vtype = (MSKsoliteme)v;

			if (!isdef_solitem(stype, vtype))
				continue;

			string vname;
			int vsize;
			getspecs_solitem(vtype, NUMVAR, NUMCON, vname, vsize);

			RowVector xx(vsize);
			double *pxx = xx.fortran_vec();
			errcatch( MSK_getsolutionslice(task,
									stype, 		/* Request current solution type. */
									vtype,		/* Which part of solution. */
									0, 			/* Index of first variable. */
									vsize, 		/* Index of last variable+1. */
									pxx));

			soltype.assign(vname, octave_value(xx));
		}

		string sname;
		getspecs_soltype(stype, sname);

		solvec.assign(sname, octave_value(soltype));
	}
}

void msk_addresponse(Octave_map &ret_val, const msk_response &res, bool overwrite=true) {

	// Construct: result -> response
	Octave_map res_vec;

	res_vec.assign("code", octave_value(res.code));
	res_vec.assign("msg", octave_value(res.msg, '\"'));

	// Append to result and overwrite if structure already exists
	// In this way errors, which calls for immediate exit, can overwrite the regular termination code..
	if (ret_val.contains("response")) {
		if (overwrite) {
			ret_val.assign("response", octave_value(res_vec));
		}
	} else {
		ret_val.assign("response", octave_value(res_vec));
	}
}

/* This function initialize the task and sets up the problem. */
void msk_setup(Task_handle &task,
					   MSKobjsensee sense, RowVector cvec, double c0,
					   SparseMatrix A,
					   RowVector blcvec, RowVector bucvec,
					   RowVector blxvec, RowVector buxvec,
					   Cell cones, int32NDArray intsubvec)
{
	int NUMANZ = A.nelem();
	int NUMCON = A.dimensions(0);
	int NUMVAR = A.dimensions(1);
	int NUMINTVAR = intsubvec.nelem();
	int NUMCONES  = cones.nelem();

	/* Matrix A */
	if (sizeof(MSKlidxt) != sizeof(octave_idx_type))
		throw msk_exception("Internal error: MSKlidxt and octave_idx_type does not agree");

	MSKlidxt *aptr = A.cidx();
	MSKlidxt *asub = A.ridx();
	double *aval = A.data();

	/* Objective costs */
	double *c = cvec.fortran_vec();

	/* Bounds on constraints. */
	MSKboundkeye bkc[NUMCON];
	double *blc = blcvec.fortran_vec();
	double *buc = bucvec.fortran_vec();
	for (int i=0; i<NUMCON; i++)
		set_boundkey(blc[i], buc[i], &bkc[i]);

	/* Bounds on variables. */
	MSKboundkeye bkx[NUMVAR];
	double *blx = blxvec.fortran_vec();
	double *bux = buxvec.fortran_vec();
	for (int i=0; i<NUMVAR; i++)
		set_boundkey(blx[i], bux[i], &bkx[i]);

	/* Index of integer variables. */
	octave_int32 *intsub = intsubvec.fortran_vec();

	// Make sure the environment is initialized
	global_env.init();

	try
	{
		/* Create the task */
		task.init(global_env, NUMCON, NUMVAR);

		/* Give MOSEK an estimate of the size of the input data.
		 * This is done to increase the speed of inputting data.
		 * However, it is optional. */
		errcatch( MSK_putmaxnumvar(task, NUMVAR) );
		errcatch( MSK_putmaxnumcon(task, NUMCON) );
		errcatch( MSK_putmaxnumanz(task, NUMANZ) );

		/* Append 'NUMCON' empty constraints.
		 * The constraints will initially have no bounds. */
		errcatch( MSK_append(task, MSK_ACC_CON, NUMCON) );

		/* Append 'NUMVAR' variables.
		 * The variables will initially be fixed at zero (x=0). */
		errcatch( MSK_append(task, MSK_ACC_VAR, NUMVAR) );

		/* Optionally add a constant term to the objective. */
		errcatch( MSK_putcfix(task, c0) );

		for (MSKidxt j=0; j<NUMVAR; ++j)
		{
			/* Set the linear term c_j in the objective.*/
			errcatch( MSK_putcj(task, j, c[j]) );

			/* Set the bounds on variable j.
			   blx[j] <= x_j <= bux[j] */
			errcatch( MSK_putbound(task,
						MSK_ACC_VAR,		/* Put bounds on variables.*/
						j, 					/* Index of variable.*/
						bkx[j],  			/* Bound key.*/
						blx[j],  			/* Numerical value of lower bound.*/
						bux[j])); 			/* Numerical value of upper bound.*/

			/* Input column j of A */
			errcatch( MSK_putavec(task,
						MSK_ACC_VAR, 		/* Input columns of A.*/
						j, 					/* Variable (column) index.*/
						aptr[j+1]-aptr[j],	/* Number of non-zeros in column j.*/
						asub+aptr[j], 		/* Pointer to row indexes of column j.*/
						aval+aptr[j])); 	/* Pointer to Values of column j.*/
		}

		/* Set the bounds on constraints.
		 * for i=1, ...,NUMCON : blc[i] <= constraint i <= buc[i] */
		for (MSKidxt i=0; i<NUMCON; ++i)
			errcatch( MSK_putbound(task,
					MSK_ACC_CON,			/* Put bounds on constraints.*/
					i,						/* Index of constraint.*/
					bkc[i],					/* Bound key.*/
					blc[i],					/* Numerical value of lower bound.*/
					buc[i]));				/* Numerical value of upper bound.*/

		/* Set the conic constraints. */
		for (MSKidxt j=0; j<NUMCONES; ++j)
			append_cone(task, cones.elem(j), j);

		/* Set the integer variables */
		for (MSKidxt j=0; j<NUMINTVAR; ++j)
			errcatch( MSK_putvartype(task, intsub[j].value()-1, MSK_VAR_TYPE_INT) );

		/* Set objective sense. */
		errcatch( MSK_putobjsense(task, sense) );

	} catch (exception const& e) {
		printoutput("An error occurred while setting up the problem.\n", typeERROR);
		throw;
	}
}

// ------------------------------
// STRUCTS OF INPUT DATA
// ------------------------------
struct options_type {
private:
	bool initialized;

public:
	// Recognised options arguments in Octave
	// TODO: Upgrade to new C++11 initialisers
	static const struct OCT_ARGS_type {

		vector<string> arglist;
		const string useparam;
		const string usesol;
		const string verbose;
		const string writebefore;
		const string writeafter;

		OCT_ARGS_type() :
			useparam("useparam"),
			usesol("usesol"),
			verbose("verbose"),
			writebefore("writebefore"),
			writeafter("writeafter")
		{
			string temp[] = {useparam, usesol, verbose, writebefore, writeafter};
			arglist = vector<string>(temp, temp + sizeof(temp)/sizeof(string));
		}
	} OCT_ARGS;


	// Data definition
	bool	useparam;
	bool 	usesol;
	double 	verbose;
	string	writebefore;
	string	writeafter;

	// Default values of optional arguments
	options_type() :
		initialized(false),

		useparam(true),
		usesol(true),
		verbose(10),
		writebefore(""),
		writeafter("")
	{}


	void OCT_read(Octave_map &arglist) {
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

//	Octave_map OCT_write() {
//		//TODO: Is this needed?
//	}
};
const options_type::OCT_ARGS_type options_type::OCT_ARGS;

// Problem input definition
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
		vector<string> arglist;
		const string sense;
		const string c;
		const string c0;
		const string A;
		const string blc;
		const string buc;
		const string blx;
		const string bux;
		const string cones;
		const string intsub;
		const string sol;
		const string iparam;
		const string dparam;
		const string sparam;
//		const string options;

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
			string temp[] = {sense, c, c0, A, blc, buc, blx, bux, cones, intsub, sol, iparam, dparam, sparam}; //options
			arglist = vector<string>(temp, temp + sizeof(temp)/sizeof(string));
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
	Cell 			cones;
	int32NDArray 	intsub;
	Octave_map 		initsol;
	Octave_map 		iparam;
	Octave_map 		dparam;
	Octave_map	 	sparam;
	options_type 	options;

	// Default values of optional arguments
	problem_type() :
		initialized(false),

		sense	(MSK_OBJECTIVE_SENSE_UNDEFINED),
		c0		(0),
		options	(options_type())
	{}


	void OCT_read(Octave_map &arglist) {
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
		sense = get_objective(sensename);

		// Objective function
		map_seek_RowVector(&c, arglist, OCT_ARGS.c);			validate_RowVector(c, OCT_ARGS.c, numvar);
		map_seek_Scalar(&c0, arglist, OCT_ARGS.c0, true);

		// Constraint and Variable Bounds
		map_seek_RowVector(&blc, arglist, OCT_ARGS.blc);		validate_RowVector(blc, OCT_ARGS.blc, numcon);
		map_seek_RowVector(&buc, arglist, OCT_ARGS.buc);		validate_RowVector(buc, OCT_ARGS.buc, numcon);
		map_seek_RowVector(&blx, arglist, OCT_ARGS.blx);		validate_RowVector(blx, OCT_ARGS.blx, numvar);
		map_seek_RowVector(&bux, arglist, OCT_ARGS.bux);		validate_RowVector(bux, OCT_ARGS.bux, numvar);

		// Cones, Integers variables and Solutions
		map_seek_Cell(&cones, arglist, OCT_ARGS.cones, true);
		map_seek_IntegerArray(&intsub, arglist, OCT_ARGS.intsub, true);
		map_seek_OctaveMap(&initsol, arglist, OCT_ARGS.sol, true);
		numcones = cones.nelem();
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

	void OCT_write(Octave_map &prob_val) {
		if (!initialized) {
			throw msk_exception("Internal error in problem_type::OCT_write, no problem was loaded");
		}
		printdebug("Started writing Octave problem output");

		// Objective sense
		prob_val.assign("sense", octave_value(getspecs_sense(sense), '\"'));

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
			prob_val.assign("cones", octave_value(cones));
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

	void MOSEK_read(Task_handle &task) {
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

			MSKboundkeye *bk = new MSKboundkeye[numcon];

			try {
				errcatch( MSK_getboundslice(task, MSK_ACC_CON, 0, numcon, bk, pblc, pbuc) );

				for (int i=0; i<numcon; i++) {
					switch (bk[i]) {
					case MSK_BK_FR:
						pbuc[i] = INFINITY;
						pblc[i] = -INFINITY;
						break;
					case MSK_BK_LO:
						pbuc[i] = INFINITY;
						break;
					case MSK_BK_UP:
						pblc[i] = -INFINITY;
						break;
					default:
						break;
					}
				}

			} catch (exception const& e) {
				delete[] bk;
				throw;
			} /* OTHERWISE */ {
				delete[] bk;
			}
		}

		// Variable bounds
		{
			printdebug("problem_type::MOSEK_read - Variable bounds");

			blx = RowVector(numvar);
			bux = RowVector(numvar);

			double *pblx = blx.fortran_vec();
			double *pbux = bux.fortran_vec();

			MSKboundkeye *bk = new MSKboundkeye[numvar];

			try {
				errcatch( MSK_getboundslice(task, MSK_ACC_VAR, 0, numvar, bk, pblx, pbux) );

				for (int i=0; i<numvar; i++) {
					switch (bk[i]) {
					case MSK_BK_FR:
						pbux[i] = INFINITY;
						pblx[i] = -INFINITY;
						break;
					case MSK_BK_LO:
						pbux[i] = INFINITY;
						break;
					case MSK_BK_UP:
						pblx[i] = -INFINITY;
						break;
					default:
						break;
					}
				}

			} catch (exception const& e) {
				delete[] bk;
				throw;
			} /* OTHERWISE */ {
				delete[] bk;
			}
		}

		// Cones
		if (numcones > 0) {
			printdebug("problem_type::MOSEK_read - Cones");

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

			char paramname[MSK_MAX_STR_LEN];
			char valuename[MSK_MAX_STR_LEN];	MSKintt value;
			for (int v=MSK_IPAR_BEGIN; v<MSK_IPAR_END; ++v) {

				// Get name of parameter
				errcatch( MSK_getparamname(task, MSK_PAR_INT_TYPE, v, paramname) );

				string paramstr = paramname;
				remove_mskprefix(paramstr, "MSK_IPAR_");

				// Get value of parameter
				errcatch( MSK_getintparam(task, (MSKiparame)v, &value) );
				errcatch( MSK_iparvaltosymnam(global_env, (MSKiparame)v, value, valuename) );

				string valuestr = valuename;
				remove_mskprefix(valuestr, "MSK_");

				// Append parameter to list
				if (!valuestr.empty())
					iparam.assign(paramstr, octave_value(valuestr, '\"'));
				else
					iparam.assign(paramstr, octave_value(value));
			}
		}

		// Double Parameters
		if (options.useparam) {
			printdebug("problem_type::MOSEK_read - Double Parameters");

			dparam = Octave_map();

			char paramname[MSK_MAX_STR_LEN];	MSKrealt value;
			for (int v=MSK_DPAR_BEGIN; v<MSK_DPAR_END; ++v) {

				// Get name of parameter
				errcatch( MSK_getparamname(task, MSK_PAR_DOU_TYPE, v, paramname) );

				string paramstr = paramname;
				remove_mskprefix(paramstr, "MSK_DPAR_");

				// Get value of parameter
				errcatch( MSK_getdouparam(task, (MSKdparame)v, &value) );

				// Append parameter to list
				dparam.assign(paramstr, octave_value(value));
			}
		}

		// String Parameters
		if (options.useparam) {
			printdebug("problem_type::MOSEK_read - String Parameters");

			sparam = Octave_map();

			char paramname[MSK_MAX_STR_LEN];	char *value;
			for (int v=MSK_SPAR_BEGIN; v<MSK_SPAR_END; ++v) {

				// Get name of parameter
				errcatch( MSK_getparamname(task, MSK_PAR_STR_TYPE, v, paramname) );

				string paramstr = paramname;
				remove_mskprefix(paramstr, "MSK_SPAR_");

				// Prepare for value of parameter
				size_t strlength;
				errcatch( MSK_getstrparam(task, (MSKsparame)v, 0, &strlength, NULL) );
				value = new char[strlength++];

				try {
					// Get value of parameter
					errcatch( MSK_getstrparam(task, (MSKsparame)v, strlength, NULL, value) );

					// Append parameter to list
					sparam.assign(paramstr, octave_value(value, '\"'));

				} catch (exception const& e) {
					delete[] value;
					throw;
				} /* OTHERWISE */ {
					delete[] value;
				}
			}
		}

		// Initial solution
		if (options.usesol) {
			printdebug("problem_type::MOSEK_read - Initial solution");

			initsol = Octave_map();
			msk_getsolution(initsol, task);
		}

		initialized = true;
	}

	void MOSEK_write(Task_handle &task) {
		if (!initialized) {
			throw msk_exception("Internal error in problem_type::MOSEK_write, no problem was loaded");
		}
		printdebug("Started writing MOSEK problem input");

		/* Set problem description */
		msk_setup(task, sense, c, c0,
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
};
const problem_type::OCT_ARGS_type problem_type::OCT_ARGS;

/* This function initialize the task and sets up the problem. */
void msk_loadproblemfile(Task_handle &task, string filepath, bool readparams) {

	// Make sure the environment is initialized
	global_env.init();

	// Initialize the task
	task.init(global_env, 0, 0);

	try
	{
		errcatch( MSK_readdata(task, const_cast<MSKCONST char*>(filepath.c_str())) );

	} catch (exception const& e) {
		printerror("An error occurred while loading up the problem from a file");
		throw;
	}
}

/* This function interrupts MOSEK if CTRL+C is caught in Octave */
static int MSKAPI mskcallback(MSKtask_t task, MSKuserhandle_t handle, MSKcallbackcodee caller) {

	if (octave_signal_caught) {
		printoutput("Interruption caught, terminating at first chance...\n", typeERROR);
		return 1;
	}
	return 0;
}

/* This function calls MOSEK and returns the solution. */
void msk_solve(Octave_map &ret_val, Task_handle &task, options_type options)
{
	//
	// STEP 1 - INITIALIZATION
	//
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

	//
	// STEP 2 - OPTIMIZATION
	//
	printdebug("msk_solve - OPTIMIZATION");
	try
	{
		/* Separate interface warnings from MOSEK output */
		if (mosek_interface_warnings > 0)
			printoutput("\n", typeWARNING);

		/* Run optimizer */
		MSKrescodee trmcode;
		errcatch( MSK_optimizetrm(task, &trmcode) );
		msk_addresponse(ret_val, msk_response(trmcode));

	} catch (exception const& e) {
		// Report that the CTRL+C interruption has been handled
		if (octave_signal_caught) {

			// TODO: FIND A WAY TO ACHIEVE SOMETHING LIKE THIS:
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

	//
	// STEP 3 - EXTRACT SOLUTION
	//
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

void reset_globalvariables() {
	mosek_interface_verbose  = NAN;   	// Declare messages as pending
	mosek_interface_warnings = 0;

	// TODO: FIND A WAY TO AVOID THIS (see previous TODO on 'octave_signal_caught')
	if (octave_signal_caught)
		OCTAVE_QUIT;
}

void init_early_exit(const char* msg) {
	// Force pending and future messages through
	if (isnan(mosek_interface_verbose)) {
		mosek_interface_verbose = typeALL;
	}
	printpendingmsg();
	printerror(msg);
	reset_globalvariables();
}

// ------------------------------
// OCTAVE CALL FUNCTIONS
// ------------------------------
DEFUN_DLD (__mosek__, args, nargout, "\
r = mosek(problem, opts)                                    \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek__ 								\n\
") {
	const string ARGNAMES[] = {"problem","options"};
	const string ARGTYPES[] = {"struct","struct"};
	const int num_min_args = 1;
	const int num_max_args = 2;

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_globalvariables();

		// Validate input arguments
		if (args.length() < num_min_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at least " + tostring(num_min_args) + ".");
		}
		if (args.length() > num_max_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at most " + tostring(num_max_args) + ".");
		}
		Octave_map arg0 = args(0).map_value();
		if (error_state) {
			throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
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

	} catch (msk_exception const& e) {
		init_early_exit(e.what());
 		msk_addresponse(ret_val, e.getresponse());
		return octave_value(ret_val);

	} catch (exception const& e) {
		init_early_exit(e.what());
		return octave_value(ret_val);
	}

	if (mosek_interface_warnings > 0) {
		printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n\n", typeWARNING);
	}

	// Clean and exit
	reset_globalvariables();
	return octave_value(ret_val);
}

DEFUN_DLD (__mosek_clean__, args, nargout, "\
mosek_clean()                                               \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_clean__                          \n\
") {
	// TODO: Should we read verbose from an 'opts' input argument?
	mosek_interface_verbose = typeALL;

	// Clean resources such as tasks before the environment!
	global_env.~Env_handle();

	reset_globalvariables();
	return empty_octave_value;

//	// TODO: Should we add the response code?
//	Octave_map ret_val;
//	msk_addresponse(ret_val, msk_response(MSK_RES_OK), false);
//	reset_globalvariables();
//	return octave_value(ret_val);
}


DEFUN_DLD (__mosek_version__, args, nargout, "\
r = mosek_version()                                         \n\
------------------------------------------------------------\n\
The use of internal functions is not encouraged.            \n\
INTERNAL FUNCTION: __mosek_version__                        \n\
") {
	MSKintt major, minor, build, revision;
	MSK_getversion(&major, &minor, &build, &revision);

	// Output
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
	const int num_min_args = 1;
	const int num_max_args = 2;

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_globalvariables();

		// Validate input arguments
		if (args.length() < num_min_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at least " + tostring(num_min_args) + ".");
		}
		if (args.length() > num_max_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at most " + tostring(num_max_args) + ".");
		}
		string arg0 = args(0).string_value();
		if (error_state) {
			throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
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
		msk_loadproblemfile(task, arg0, probin.options.useparam);

		// Read the problem from MOSEK
		probin.MOSEK_read(task);

		// Write the problem to Octave
		Octave_map prob_val;
		probin.OCT_write(prob_val);
		ret_val.assign("prob", octave_value(prob_val));

		// Add the response code
		msk_addresponse(ret_val, msk_response(MSK_RES_OK), false);

	} catch (msk_exception const& e) {
		init_early_exit(e.what());
		msk_addresponse(ret_val, e.getresponse());
		return octave_value(ret_val);

	} catch (exception const& e) {
		init_early_exit(e.what());
		return octave_value(ret_val);
	}

	if (mosek_interface_warnings > 0) {
		printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n", typeWARNING);
	}

	// Clean and exit
	reset_globalvariables();
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
	const int num_min_args = 2;
	const int num_max_args = 3;

	// Create structure for returned data
	Octave_map ret_val;

	try {
		// Start the program
		reset_globalvariables();

		// Validate input arguments
		if (args.length() < num_min_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at least " + tostring(num_min_args) + ".");
		}
		if (args.length() > num_max_args) {
			print_usage();
			throw msk_exception("Wrong number of input arguments, should be at most " + tostring(num_max_args) + ".");
		}
		Octave_map arg0 = args(0).map_value();
		if (error_state) {
			throw msk_exception("Input argument " + ARGNAMES[0] + " should be a " + ARGTYPES[0] + ".");
		}
		string arg1 = args(1).string_value();
		if (error_state) {
			throw msk_exception("Input argument " + ARGNAMES[1] + " should be a " + ARGTYPES[1] + ".");
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

		// Set export-parameters for whether to write any solution loaded into MOSEK
		if (probin.options.usesol) {
			errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_ON) );
		} else {
			errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_SOLUTIONS, MSK_OFF) );
		}

		// Set export-parameters for whether to write all parameters
		if (probin.options.useparam) {
			errcatch( MSK_putintparam(task, MSK_IPAR_WRITE_DATA_PARAM,MSK_ON) );
			errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PARAMETERS,MSK_ON) );
		} else {
			errcatch( MSK_putintparam(task, MSK_IPAR_WRITE_DATA_PARAM,MSK_OFF) );
			errcatch( MSK_putintparam(task, MSK_IPAR_OPF_WRITE_PARAMETERS,MSK_OFF) );
		}

		// Write to filepath model (filetypes: .lp, .mps, .opf, .mbt)
		errcatch( MSK_writedata(task, const_cast<MSKCONST char*>(arg1.c_str())) );

		// Add the response code
		msk_addresponse(ret_val, msk_response(MSK_RES_OK), false);

	} catch (msk_exception const& e) {
		init_early_exit(e.what());
		msk_addresponse(ret_val, e.getresponse());
		return octave_value(ret_val);

	} catch (exception const& e) {
		init_early_exit(e.what());
		return octave_value(ret_val);
	}

	if (mosek_interface_warnings > 0) {
		printoutput("The Octave-to-MOSEK interface completed with " + tostring(mosek_interface_warnings) + " warning(s)\n", typeWARNING);
	}

	// Clean and exit
	reset_globalvariables();
	return octave_value(ret_val);
}
