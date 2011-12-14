#ifndef OMSK_UTILS_OCTAVE_H_
#define OMSK_UTILS_OCTAVE_H_

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/Matrix.h>

#include <string>
#include <vector>

// Check value objects
bool isEmpty(octave_value& obj);
bool isEmpty(Octave_map& obj);
bool isEmpty(SparseMatrix& obj);

template<class T>
bool isEmpty(Array<T>& obj) {
	return (obj.nelem() == 0);
}


// Seek octave_value in list
void map_seek_Value(octave_value *out, Octave_map map, std::string name, bool optional=false);

// Seek other types in list
void map_seek_OctaveMap(Octave_map *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_Cell(Cell *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_SparseMatrix(SparseMatrix *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_RowVector(RowVector *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_IntegerArray(int32NDArray *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_Scalar(double *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_String(std::string *out, Octave_map& map, std::string name, bool optional=false);
void map_seek_Boolean(bool *out, Octave_map& map, std::string name, bool optional=false);

// Validate other types fetched from list
void validate_OctaveMap(Octave_map& object, std::string name, std::vector<std::string> keywords, bool optional=false);
void validate_Cell(Cell &object, std::string name, int nelem, bool optional=false);
void validate_SparseMatrix(SparseMatrix& object, std::string name, int nrows, int ncols, bool optional=false);
void validate_RowVector(RowVector& object, std::string name, int nrows, bool optional=false);
void validate_IntegerArray(int32NDArray& object, std::string name, int nrows, bool optional=false);


#endif /* OMSK_UTILS_OCTAVE_H_ */
