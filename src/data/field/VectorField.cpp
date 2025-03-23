//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorField.h"

namespace artemis::data {

void _VectorField32::init(const RegularGrid& grid) {
	v.resize(grid.cell_count());
}

void _VectorField64::init(const RegularGrid& grid) {
	v.resize(grid.cell_count());
}

VectorField::VectorField(const RegularGrid& g, ScalarType t) {
	grid = g;
	type = t;
	if (type == ScalarType::Float32)
		v32.init(grid);
	else if (type == ScalarType::Float64)
		v64.init(grid);
}

VectorField::VectorField() : VectorField(RegularGrid(), ScalarType::None) {}

dvec3 VectorField::value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return dvec3(v32.v[grid.cell_index(i, j, k)]);
	if (type == ScalarType::Float64)
		return v64.v[grid.cell_index(i, j, k)];
	return {0,0,0};
}

void VectorField::set(int i, int j, int k, const dvec3& vv) {
	if (type == ScalarType::Float32)
		v32.v[grid.cell_index(i, j, k)] = vv.to32();
	else if (type == ScalarType::Float64)
		v64.v[grid.cell_index(i, j, k)] = vv;
}

vec3 VectorField::value32(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return v32.v[grid.cell_index(i, j, k)];
	if (type == ScalarType::Float64)
		return v64.v[grid.cell_index(i, j, k)].to32();
	return {0,0,0};
}

void VectorField::set32(int i, int j, int k, const vec3& vv) {
	if (type == ScalarType::Float32)
		v32.v[grid.cell_index(i, j, k)] = vv;
	else if (type == ScalarType::Float64)
		v64.v[grid.cell_index(i, j, k)] = dvec3(vv);
}

template<class T>
void list_add(T& a, const T& b) {
	for (int i=0; i<a.num; i++)
		a[i] += b[i];
}

template<class T>
void list_sub(T& a, const T& b) {
	for (int i=0; i<a.num; i++)
		a[i] -= b[i];
}

template<class T>
void list_mul_single(T& a, float s) {
	for (int i=0; i<a.num; i++)
		a[i] *= s;
}

void VectorField::operator+=(const VectorField& o) {
	if (o.type != type)
		return;
	if (type == ScalarType::Float32)
		list_add(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		list_add(v64.v, o.v64.v);
}

VectorField VectorField::operator+(const VectorField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void VectorField::operator-=(const VectorField& o) {
	if (o.type != type)
		return;
	if (type == ScalarType::Float32)
		list_sub(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		list_sub(v64.v, o.v64.v);
}

VectorField VectorField::operator-(const VectorField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void VectorField::operator*=(float o) {
	if (type == ScalarType::Float32)
		list_mul_single(v32.v, o);
	else if (type == ScalarType::Float64)
		list_mul_single(v64.v, o);
}

VectorField VectorField::operator*(float o) const {
	auto r = *this;
	r *= o;
	return r;
}

VectorField VectorField::componentwise_product(const VectorField& o) const {
	auto r = VectorField(grid, o.type);
	if (type != o.type)
		return r;
	auto times32 = [] (const vec3& a, const vec3& b) {
		return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
	};
	auto times = [] (const dvec3& a, const dvec3& b) {
		return dvec3(a.x * b.x, a.y * b.y, a.z * b.z);
	};
	if (type == ScalarType::Float32) {
		for (int i=0; i<v32.v.num; i++)
			r.v32.v[i] = times32(v32.v[i], o.v32.v[i]);
	} else if (type == ScalarType::Float64) {
		for (int i=0; i<v64.v.num; i++)
			r.v64.v[i] = times(v64.v[i], o.v64.v[i]);
	}
	return r;
}



}
