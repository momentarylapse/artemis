//
// Created by michi on 9/1/25.
//

#include "Field.h"

namespace artemis::data {


void Field::init(const RegularGrid& _grid, ScalarType _type, int _components, SamplingMode mode) {
	grid = _grid;
	type = _type;
	components = _components;
	stride = components * scalar_size(type);
	sampling_mode = mode;
	n = grid.count(sampling_mode);
	data.resize(n * stride);
}

double Field::value(int index, int n) const {
	if (type == ScalarType::Float32)
		return ((float*)at(index))[n];
	if (type == ScalarType::Float64)
		return ((double*)at(index))[n];
	return 0.0;
}


void Field::set(int index, int n, double vv) {
	if (type == ScalarType::Float32)
		((float*)at(index))[n] = (float)vv;
	else if (type == ScalarType::Float64)
		((double*)at(index))[n] = vv;
}

double Field::_value(int i, int j, int k, int n) const {
	if (type == ScalarType::Float32)
		return ((float*)at(i, j, k))[n];
	if (type == ScalarType::Float64)
		return ((double*)at(i, j, k))[n];
	return 0;
}

void Field::_set(int i, int j, int k, int n, double vv) {
	if (type == ScalarType::Float32)
		((float*)at(i, j, k))[n] = (float)vv;
	else if (type == ScalarType::Float64)
		((double*)at(i, j, k))[n] = vv;
}

/*void Field::operator=(const Field &o) {
	grid = o.grid;
	type = o.type;
	components = o.components;
	stride = components * scalar_size(type);
	sampling_mode = o.sampling_mode;
	n = grid.count(sampling_mode);
	//data.resize(n * stride);
	data = o.data;
	msg_write(format("=   FIELD %d  %d  %d  %d", n, components, stride, data.num));
}*/


}
