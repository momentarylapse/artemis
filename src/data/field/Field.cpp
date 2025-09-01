//
// Created by michi on 9/1/25.
//

#include "Field.h"
#include <lib/ygraphics/graphics-impl.h>

namespace artemis::data {

Field::Field() = default;
Field::~Field() = default;


void Field::init(const RegularGrid& _grid, ScalarType _type, int _components, SamplingMode mode) {
	grid = _grid;
	type = _type;
	components = _components;
	stride = components * scalar_size(type);
	sampling_mode = mode;
	n = grid.count(sampling_mode);
	data.resize(n * stride);
	location = Location::CPU;
}

void Field::_copy_cpu_to_gpu() const {
	if (!buffer)
		buffer = new ygfx::ShaderStorageBuffer(stride * n);
	buffer->update_array(data);
}

void Field::_copy_gpu_to_cpu() const {
	data.resize(n * stride);
	buffer->read_array(data);
}


void Field::begin_edit_gpu() {
	if (location == Location::Both) {
		location = Location::GPU;
	} else if (location == Location::CPU) {
		_copy_cpu_to_gpu();
		location = Location::GPU;
	}
}

void Field::begin_edit_cpu() {
	if (location == Location::Both) {
		location = Location::CPU;
	} else if (location == Location::GPU) {
		_copy_gpu_to_cpu();
		location = Location::CPU;
	}
}

void Field::begin_read_cpu() const {
	if (location == Location::GPU) {
		_copy_gpu_to_cpu();
		location = Location::CPU;
	}
}

void Field::begin_read_gpu() const {
	if (location == Location::CPU) {
		_copy_cpu_to_gpu();
		location = Location::Both;
	}
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

Field& Field::operator=(const Field &o) {
	o.begin_read_cpu();
	grid = o.grid;
	type = o.type;
	components = o.components;
	stride = components * scalar_size(type);
	sampling_mode = o.sampling_mode;
	n = grid.count(sampling_mode);
	//data.resize(n * stride);
	data = o.data;
	location = Location::CPU;
	/*if (o.buffer) {
		buffer = new ygfx::ShaderStorageBuffer(stride * n);
	}*/
	return *this;
}


}
