//
// Created by michi on 8/14/25.
//

#include "MultiComponentField.h"
#include <processing/helper/GlobalThreadPool.h>

namespace artemis::data {

MultiComponentField::MultiComponentField(const RegularGrid& g, ScalarType t, SamplingMode s, int n) {
	grid = g;
	type = t;
	sampling_mode = s;
	components = n;
	if (type == ScalarType::Float32)
		v32.init(grid, components, sampling_mode);
	else if (type == ScalarType::Float64)
		v64.init(grid, components, sampling_mode);
}

MultiComponentField::MultiComponentField() : MultiComponentField(RegularGrid(), ScalarType::None, SamplingMode::PerCell, 1) {}

double MultiComponentField::value(int index, int n) const {
	if (type == ScalarType::Float32)
		return (double)v32._at(index)[n];
	if (type == ScalarType::Float64)
		return v64._at(index)[n];
	return 0;
}

Array<double> MultiComponentField::values(int index) const {
	Array<double> r;
	for (int i=0; i<components; i++)
		r.add(value(index, i));
	return r;
}


void MultiComponentField::set(int index, int n, double vv) {
	if (type == ScalarType::Float32)
		v32._at(index)[n] = (float)vv;
	else if (type == ScalarType::Float64)
		v64._at(index)[n] = vv;
}

double MultiComponentField::_value(int i, int j, int k, int n) const {
	if (type == ScalarType::Float32)
		return (double)v32.at(grid, sampling_mode, i, j, k)[n];
	if (type == ScalarType::Float64)
		return v64.at(grid, sampling_mode, i, j, k)[n];
	return 0;
}

void MultiComponentField::_set(int i, int j, int k, int n, double vv) {
	if (type == ScalarType::Float32)
		v32.at(grid, sampling_mode, i, j, k)[n] = (float)vv;
	else if (type == ScalarType::Float64)
		v64.at(grid, sampling_mode, i, j, k)[n] = vv;
}

template<class T>
void list_add(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] += b[i];
	}, 1000);
}

template<class T>
void list_sub(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] -= b[i];
	}, 1000);
}

template<class T>
void list_mul_single(T& a, double s) {
	processing::pool::run(a.num, [&a, s] (int i) {
		a[i] *= s;
	}, 1000);
}

void MultiComponentField::operator+=(const MultiComponentField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		list_add(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		list_add(v64.v, o.v64.v);
}

MultiComponentField MultiComponentField::operator+(const MultiComponentField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void MultiComponentField::operator-=(const MultiComponentField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		list_sub(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		list_sub(v64.v, o.v64.v);
}

MultiComponentField MultiComponentField::operator-(const MultiComponentField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void MultiComponentField::operator*=(double o) {
	if (type == ScalarType::Float32)
		list_mul_single(v32.v, o);
	else if (type == ScalarType::Float64)
		list_mul_single(v64.v, o);
}

MultiComponentField MultiComponentField::operator*(double o) const {
	auto r = *this;
	r *= o;
	return r;
}

MultiComponentField MultiComponentField::componentwise_product(const MultiComponentField& o) const {
	auto r = MultiComponentField(grid, type, sampling_mode, min(components, o.components));
	if (type != o.type or sampling_mode != o.sampling_mode or components != o.components)
		return r;
	if (type == ScalarType::Float32) {
		r.v32.cwise_product(v32, o.v32);
	} else if (type == ScalarType::Float64) {
		r.v64.cwise_product(v64, o.v64);
	}
	return r;
}



}
