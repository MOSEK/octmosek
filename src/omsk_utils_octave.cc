#include "omsk_utils_octave.h"

#include "omsk_msg_base.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// ------------------------------
// CHECK VALUES
// ------------------------------

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
bool isEmpty(SparseMatrix& obj) {
	return (obj.nnz() == 0);
}


// ------------------------------
// Seek object: octave_value
// ------------------------------
void map_seek_Value(octave_value *out, Octave_map map, string name, bool optional)
{
	Octave_map::const_iterator iter = map.seek(name);
	if (iter != map.end() && map.contents(iter).length() >= 1)
		*out = map.contents(iter)(0);
	else if (!optional)
		throw msk_exception("An expected variable named '" + name + "' was not found");
}

// ------------------------------
// Seek object: Octave_map
// ------------------------------
void map_seek_OctaveMap(Octave_map *out, Octave_map& map, string name, bool optional)
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
void validate_OctaveMap(Octave_map& object, string name, vector<string> keywords, bool optional)
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

// ------------------------------
// Seek object: Cell
// ------------------------------
void map_seek_Cell(Cell *out, Octave_map& map, string name, bool optional)
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
void validate_Cell(Cell &object, string name, int nelem, bool optional)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nelem)
		throw msk_exception("Cell \"" + name + "\" has the wrong dimensions");
}

// ------------------------------
// Seek object: SparseMatrix
// ------------------------------
void map_seek_SparseMatrix(SparseMatrix *out, Octave_map& map, string name, bool optional)
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
void validate_SparseMatrix(SparseMatrix& object, string name, int nrows, int ncols, bool optional)
{
	if (optional && isEmpty(object))
		return;

	if (object.dim1() != nrows || object.dim2() != ncols)
		throw msk_exception("Sparse Matrix \"" + name + "\" has the wrong dimensions");
}

// ------------------------------
// Seek object: Row Vector (use IntegerArray for integer-typed vectors)
// ------------------------------
void map_seek_RowVector(RowVector *out, Octave_map& map, string name, bool optional)
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
void validate_RowVector(RowVector& object, string name, int nrows, bool optional)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nrows)
		throw msk_exception("Vector \"" + name + "\" has the wrong dimensions");
}

// ------------------------------
// Seek object: IntegerArray
// ------------------------------
void map_seek_IntegerArray(int32NDArray *out, Octave_map& map, string name, bool optional)
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
void validate_IntegerArray(int32NDArray& object, string name, int nrows, bool optional)
{
	if (optional && isEmpty(object))
		return;

	if (object.nelem() != nrows)
		throw msk_exception("Integer Vector \"" + name + "\" has the wrong dimensions");
}

// ------------------------------
// Seek object: Scalar
// ------------------------------
void map_seek_Scalar(double *out, Octave_map& map, string name, bool optional)
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

// ------------------------------
// Seek object: String
// ------------------------------
void map_seek_String(string *out, Octave_map& map, string name, bool optional)
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

// ------------------------------
// Seek object: BOOLEAN
// ------------------------------
void map_seek_Boolean(bool *out, Octave_map& map, string name, bool optional)
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
