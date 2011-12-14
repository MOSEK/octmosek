#include "omsk_msg_base.h"

#include <string>
#include <vector>

using std::string;
using std::vector;


// Global initialization
double mosek_interface_verbose  = NAN;				// Declare messages as pending
int    mosek_interface_warnings = 0;				// Start warning count from zero

const octave_value empty_octave_value = octave_value(Octave_map());


// ------------------------------
// PRINTING SYSTEM
// ------------------------------
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

void delete_all_pendingmsg() {
	mosek_pendingmsg_str.clear();
	mosek_pendingmsg_type.clear();
}

void printpendingmsg() {
	if (mosek_pendingmsg_str.size() != mosek_pendingmsg_type.size()) {
		delete_all_pendingmsg();
		throw msk_exception("Error in handling of pending messages");
	}

	for (size_t i=0; i<mosek_pendingmsg_str.size(); i++) {
		printoutput(mosek_pendingmsg_str[i], mosek_pendingmsg_type[i]);
	}
	delete_all_pendingmsg();
}


// ------------------------------
// BASIC TYPE MANIPULATION
// ------------------------------

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
