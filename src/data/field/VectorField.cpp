//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorField.h"
#include <processing/helper/GlobalThreadPool.h>

namespace artemis::data {

VectorField::VectorField(const RegularGrid& g, ScalarType t, SamplingMode s) {
	grid = g;
	type = t;
	sampling_mode = s;
	if (type == ScalarType::Float32)
		v32.init(grid, 3, sampling_mode);
	else if (type == ScalarType::Float64)
		v64.init(grid, 3, sampling_mode);
}

VectorField::VectorField() : VectorField(RegularGrid(), ScalarType::None, SamplingMode::PerCell) {}

dvec3 VectorField::value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return dvec3(*(vec3*)v32.at(grid, sampling_mode, i, j, k));
	if (type == ScalarType::Float64)
		return *(dvec3*)v64.at(grid, sampling_mode, i, j, k);
	return {0,0,0};
}

void VectorField::set(int i, int j, int k, const dvec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)v32.at(grid, sampling_mode, i, j, k) = vv.to32();
	else if (type == ScalarType::Float64)
		*(dvec3*)v64.at(grid, sampling_mode, i, j, k) = vv;
}

vec3 VectorField::value32(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return *(vec3*)v32.at(grid, sampling_mode, i, j, k);
	if (type == ScalarType::Float64)
		return ((dvec3*)v64.at(grid, sampling_mode, i, j, k))->to32();
	return {0,0,0};
}

void VectorField::set32(int i, int j, int k, const vec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)v32.at(grid, sampling_mode, i, j, k) = vv;
	else if (type == ScalarType::Float64)
		*(dvec3*)v64.at(grid, sampling_mode, i, j, k) = dvec3(vv);
}

template<class T>
void list_add(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] += b[i];
	}, 1000);
//	for (int i=0; i<a.num; i++)
//		a[i] += b[i];
}

template<class T>
void list_sub(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] -= b[i];
	}, 1000);
//	for (int i=0; i<a.num; i++)
//		a[i] -= b[i];
}

template<class T>
void list_mul_single(T& a, float s) {
	processing::pool::run(a.num, [&a, s] (int i) {
		a[i] *= s;
	}, 1000);
//	for (int i=0; i<a.num; i++)
//		a[i] *= s;
}

void VectorField::operator+=(const VectorField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
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
	if (o.type != type or sampling_mode != o.sampling_mode)
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
	auto r = VectorField(grid, type, sampling_mode);
	if (type != o.type or sampling_mode != o.sampling_mode)
		return r;
	if (type == ScalarType::Float32) {
		r.v32.cwise_product(v32, o.v32);
	} else if (type == ScalarType::Float64) {
		r.v64.cwise_product(v64, o.v64);
	}
	return r;
}



}
