//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"
#include <processing/helper/GlobalThreadPool.h>
#include <lib/os/msg.h>

namespace artemis::data {

ScalarField::ScalarField(const RegularGrid& g, ScalarType t, SamplingMode s) {
	grid = g;
	type = t;
	sampling_mode = s;
	if (type == ScalarType::Float32)
		v32.init(grid, 1, sampling_mode);
	else if (type == ScalarType::Float64)
		v64.init(grid, 1, sampling_mode);
}

ScalarField::ScalarField() : ScalarField(RegularGrid(), ScalarType::None, SamplingMode::PerCell) {}

double ScalarField::value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return (double)*v32.at(grid, sampling_mode, i, j, k);
	if (type == ScalarType::Float64)
		return *v64.at(grid, sampling_mode, i, j, k);
	return 0.0;
}

void ScalarField::set(int i, int j, int k, double f) {
	if (type == ScalarType::Float32)
		*v32.at(grid, sampling_mode, i, j, k) = (float)f;
	else if (type == ScalarType::Float64)
		*v64.at(grid, sampling_mode, i, j, k) = f;
}

float ScalarField::value32(int i, int j, int k) const {
	return (float)value(i, j, k);
}

void ScalarField::set32(int i, int j, int k, float f) {
	set(i, j, k, (double)f);
}

void ScalarField::_set(int index, double f) {
	if (type == ScalarType::Float32)
		*v32._at(index) = (float)f;
	else if (type == ScalarType::Float64)
		*v64._at(index) = f;
}

void ScalarField::_set32(int index, float f) {
	_set(index, (double)f);
}


template<class T>
void s_list_add(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] += b[i];
	}, 1000);
}

template<class T>
void s_list_sub(T& a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] -= b[i];
	}, 1000);
}

template<class T>
void s_list_mul_single(T& a, double s) {
	processing::pool::run(a.num, [&a, s] (int i) {
		a[i] *= s;
	}, 10000);
}

void ScalarField::operator+=(const ScalarField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		s_list_add(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		s_list_add(v64.v, o.v64.v);
}

ScalarField ScalarField::operator+(const ScalarField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void ScalarField::operator-=(const ScalarField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		s_list_sub(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		s_list_sub(v64.v, o.v64.v);
}

ScalarField ScalarField::operator-(const ScalarField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void ScalarField::operator*=(double o) {
	if (type == ScalarType::Float32)
		s_list_mul_single(v32.v, o);
	else if (type == ScalarType::Float64)
		s_list_mul_single(v64.v, o);
}

ScalarField ScalarField::operator*(double o) const {
	auto r = *this;
	r *= o;
	return r;
}

}
