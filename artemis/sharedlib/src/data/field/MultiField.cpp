//
// Created by michi on 8/14/25.
//

#include "MultiField.h"
#include "ScalarField.h"
#include <processing/helper/GlobalThreadPool.h>

namespace artemis::data {

MultiField::MultiField(const RegularGrid& g, ScalarType t, SamplingMode s, int n) {
	init(g, t, n, s);
}

MultiField::MultiField() : MultiField(RegularGrid(), ScalarType::None, SamplingMode::PerCell, 1) {}

MultiField::MultiField(const MultiField &other) {
	*this = other;
}

MultiField::~MultiField() = default;



Array<double> MultiField::values(int index) const {
	Array<double> r;
	for (int i=0; i<components; i++)
		r.add(value(index, i));
	return r;
}

template<class T>
void list_iadd(Field& a, const Field& b) {
	processing::pool::run(a.n, [&a, &b] (int i) {
		auto pa = (T*)a.at(i);
		auto pb = (T*)b.at(i);
		for (int k=0; k<a.components; k++)
			pa[k] += pb[k];
	}, 1024);
}

template<class T>
void list_isub(Field& a, const Field& b) {
	processing::pool::run(a.n, [&a, &b] (int i) {
		auto pa = (T*)a.at(i);
		auto pb = (T*)b.at(i);
		for (int k=0; k<a.components; k++)
			pa[k] -= pb[k];
	}, 1024);
}

template<class T>
void list_imul_single(Field& a, double s) {
	processing::pool::run(a.n, [&a, s] (int i) {
		auto pa = (T*)a.at(i);
		for (int k=0; k<a.components; k++)
			pa[k] *= s;
	}, 1024);
}

template<class T>
void list_mul(Field& r, const Field& a, const Field& b) {
	processing::pool::run(a.n, [&r, &a, &b] (int i) {
		auto pr = (T*)r.at(i);
		auto pa = (T*)a.at(i);
		auto pb = (T*)b.at(i);
		for (int k=0; k<a.components; k++)
			pr[k] = pa[k] * pb[k];
	}, 1024);
}

void MultiField::operator=(const MultiField &o) {
	this->Field::operator=(o);
}


void MultiField::operator+=(const MultiField& o) {
	if (o.type != type or o.n != n or o.components != components)
		return;
	o.begin_read_cpu();
	begin_edit_cpu();
	if (type == ScalarType::Float32)
		list_iadd<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_iadd<double>(*this, o);
}

MultiField MultiField::operator+(const MultiField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void MultiField::operator-=(const MultiField& o) {
	if (o.type != type or o.n != n or o.components != components)
		return;
	o.begin_read_cpu();
	begin_edit_cpu();
	if (type == ScalarType::Float32)
		list_isub<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_isub<double>(*this, o);
}

MultiField MultiField::operator-(const MultiField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void MultiField::operator*=(double o) {
	begin_edit_cpu();
	if (type == ScalarType::Float32)
		list_imul_single<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_imul_single<double>(*this, o);
}

MultiField MultiField::operator*(double o) const {
	auto r = *this;
	r *= o;
	return r;
}

ScalarField MultiField::get_component(int component) const {;
	ScalarField s(grid, type, sampling_mode);
	if (component < 0 or component >= components)
		return s;
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v32.v.num; i++) {
			s.v32.v[i] = (float)value(i, component);
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v64.v.num; i++) {
			s.v64.v[i] = value(i, component);
		}
	}
	return s;
}

MultiField MultiField::componentwise_product(const MultiField& o) const {
	auto r = MultiField(grid, type, sampling_mode, min(components, o.components));
	if (type != o.type or sampling_mode != o.sampling_mode or n != o.n or components != o.components)
		return r;
	o.begin_read_cpu();
	begin_read_cpu();
	if (type == ScalarType::Float32)
		list_mul<float>(r, *this, o);
	else if (type == ScalarType::Float64)
		list_mul<double>(r, *this, o);
	return r;
}



}
