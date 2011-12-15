#include "omsk_utils_mosek.h"

#include "omsk_utils_octave.h"
#include "omsk_utils_interface.h"
#include "omsk_obj_mosek.h"

#include <string>
#include <exception>

//FIXME
#include <memory>
#include <vector>

using std::string;
using std::exception;

using std::auto_ptr;
using std::vector;


// ------------------------------
// MOSEK-UTILS
// ------------------------------

/* This function explains the solution types of MOSEK. */
string get_objective(MSKobjsensee sense)
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
MSKobjsensee get_mskobjective(string sense)
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

void get_boundvalues(MSKtask_t task, double *lower, double* upper, MSKaccmodee boundtype, MSKintt numbounds)
{
	auto_array<MSKboundkeye> bk( new MSKboundkeye[numbounds] );

	// Get bound keys from MOSEK
	errcatch( MSK_getboundslice(task, boundtype, 0, numbounds, bk, lower, upper) );

	for (MSKintt i=0; i<numbounds; i++) {
		switch (bk[i]) {
			case MSK_BK_FR:
				lower[i] = -INFINITY;
				upper[i] = INFINITY;
				break;
			case MSK_BK_LO:
				upper[i] = INFINITY;
				break;
			case MSK_BK_UP:
				lower[i] = -INFINITY;
				break;
			default:
				break;
		}
	}
}

void get_mskparamtype(MSKtask_t task, string type, string name, MSKparametertypee *ptype, MSKintt *pidx)
{
	// Convert name to mosek input with correct prefix
	strtoupper(name);
	if (type == "iparam") append_mskprefix(name, "MSK_IPAR_"); else
	if (type == "dparam") append_mskprefix(name, "MSK_DPAR_"); else
	if (type == "sparam") append_mskprefix(name, "MSK_SPAR_"); else
		throw msk_exception("A parameter type was not recognized");

	errcatch( MSK_whichparam(task, const_cast<MSKCONST char*>(name.c_str()), ptype, pidx) );
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

	MSKparametertypee ptype;
	MSKintt pidx;
	get_mskparamtype(task, type, name, &ptype, &pidx);

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

void get_int_parameters(Octave_map &paramvec, MSKtask_t task)
{
	char paramname[MSK_MAX_STR_LEN];
	char valuename[MSK_MAX_STR_LEN];	MSKintt value;
	for (int v=MSK_IPAR_BEGIN; v<MSK_IPAR_END; ++v) {

		// Get name of parameter
		errcatch( MSK_getparamname(task, MSK_PAR_INT_TYPE, v, paramname) );

		string paramstr = paramname;
		remove_mskprefix(paramstr, "MSK_IPAR_");

		// Get value of parameter
		errcatch( MSK_getintparam(task, static_cast<MSKiparame>(v), &value) );
		errcatch( MSK_iparvaltosymnam(global_env, static_cast<MSKiparame>(v), value, valuename) );

		string valuestr = valuename;
		remove_mskprefix(valuestr, "MSK_");

		// Append parameter to list
		if (!valuestr.empty())
			paramvec.assign(paramstr, octave_value(valuestr, '\"'));
		else
			paramvec.assign(paramstr, octave_value(value));
	}
}

void get_dou_parameters(Octave_map &paramvec, MSKtask_t task)
{
	char paramname[MSK_MAX_STR_LEN];	MSKrealt value;
	for (int v=MSK_DPAR_BEGIN; v<MSK_DPAR_END; ++v) {

		// Get name of parameter
		errcatch( MSK_getparamname(task, MSK_PAR_DOU_TYPE, v, paramname) );

		string paramstr = paramname;
		remove_mskprefix(paramstr, "MSK_DPAR_");

		// Get value of parameter
		errcatch( MSK_getdouparam(task, static_cast<MSKdparame>(v), &value) );

		// Append parameter to list
		paramvec.assign(paramstr, octave_value(value));
	}
}

void get_str_parameters(Octave_map &paramvec, MSKtask_t task)
{
	char paramname[MSK_MAX_STR_LEN];
	for (int v=MSK_SPAR_BEGIN; v<MSK_SPAR_END; ++v) {

		// Get name of parameter
		errcatch( MSK_getparamname(task, MSK_PAR_STR_TYPE, v, paramname) );

		string paramstr = paramname;
		remove_mskprefix(paramstr, "MSK_SPAR_");

		// Prepare for value of parameter by retrieving length
		size_t strlength;
		errcatch( MSK_getstrparam(task, static_cast<MSKsparame>(v), 0, &strlength, NULL) );

		// Terminating null-character not counted by 'MSK_getstrparam'
		++strlength;

		// Get value of parameter
		auto_array<char> value ( new char[strlength] );
		errcatch( MSK_getstrparam(task, static_cast<MSKsparame>(v), strlength, NULL, value) );

		// Append parameter to list
		paramvec.assign(paramstr, octave_value(value, '\"'));
	}
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
			auto_array<MSKstakeye> mskskc ( new MSKstakeye[NUMCON] );

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
			auto_array<MSKstakeye> mskskx ( new MSKstakeye[NUMVAR] );

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

/* Initialise the task and load problem from arguments */
void msk_loadproblem(Task_handle &task,
					   MSKobjsensee sense, RowVector cvec, double c0,
					   SparseMatrix &A,
					   RowVector blcvec, RowVector bucvec,
					   RowVector blxvec, RowVector buxvec,
					   conicSOC_type &cones, int32NDArray &intsubvec)
{
	int NUMANZ = A.nelem();
	int NUMCON = A.dimensions(0);
	int NUMVAR = A.dimensions(1);
	int NUMINTVAR = intsubvec.nelem();

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
		cones.MOSEK_write(task);

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

