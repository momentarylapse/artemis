//
// Created by michi on 8/14/25.
//

#include "MultiComponentField.h"
#include <processing/helper/GlobalThreadPool.h>

namespace artemis::data {

MultiComponentField::MultiComponentField(const RegularGrid& g, ScalarType t, SamplingMode s, int n) {
	init(g, t, n, s);
}

MultiComponentField::MultiComponentField() : MultiComponentField(RegularGrid(), ScalarType::None, SamplingMode::PerCell, 1) {}

Array<double> MultiComponentField::values(int index) const {
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

void MultiComponentField::operator+=(const MultiComponentField& o) {
	if (o.type != type or o.n != n or o.components != components)
		return;
	if (type == ScalarType::Float32)
		list_iadd<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_iadd<double>(*this, o);
}

MultiComponentField MultiComponentField::operator+(const MultiComponentField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void MultiComponentField::operator-=(const MultiComponentField& o) {
	if (o.type != type or o.n != n or o.components != components)
		return;
	if (type == ScalarType::Float32)
		list_isub<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_isub<double>(*this, o);
}

MultiComponentField MultiComponentField::operator-(const MultiComponentField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void MultiComponentField::operator*=(double o) {
	if (type == ScalarType::Float32)
		list_imul_single<float>(*this, o);
	else if (type == ScalarType::Float64)
		list_imul_single<double>(*this, o);
}

MultiComponentField MultiComponentField::operator*(double o) const {
	auto r = *this;
	r *= o;
	return r;
}

MultiComponentField MultiComponentField::componentwise_product(const MultiComponentField& o) const {
	auto r = MultiComponentField(grid, type, sampling_mode, min(components, o.components));
	if (type != o.type or sampling_mode != o.sampling_mode or n != o.n or components != o.components)
		return r;
	if (type == ScalarType::Float32)
		list_mul<float>(r, *this, o);
	else if (type == ScalarType::Float64)
		list_mul<double>(r, *this, o);
	return r;
}



}
