//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorField.h"
#include <processing/helper/GlobalThreadPool.h>

#include "ScalarField.h"
#include "lib/os/msg.h"

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

void VectorField::_set(int index, const dvec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)v32._at(index) = vv.to32();
	else if (type == ScalarType::Float64)
		*(dvec3*)v64._at(index) = vv;
}

vec3 VectorField::value32(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return *(vec3*)v32.at(grid, sampling_mode, i, j, k);
	if (type == ScalarType::Float64)
		return ((dvec3*)v64.at(grid, sampling_mode, i, j, k))->to32();
	return {0,0,0};
}

vec3 VectorField::_value32(int index) const {
	if (type == ScalarType::Float32)
		return *(vec3*)v32._at(index);
	if (type == ScalarType::Float64)
		return ((dvec3*)v64._at(index))->to32();
	return {0,0,0};
}

void VectorField::set32(int i, int j, int k, const vec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)v32.at(grid, sampling_mode, i, j, k) = vv;
	else if (type == ScalarType::Float64)
		*(dvec3*)v64.at(grid, sampling_mode, i, j, k) = dvec3(vv);
}

void VectorField::_set32(int index, const vec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)v32._at(index) = vv;
	else if (type == ScalarType::Float64)
		*(dvec3*)v64._at(index) = dvec3(vv);
}

template<class T>
void v_list_add(T& a, const T& b) {
#if 1
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] += b[i];
	}, 1000);
#else
	for (int i=0; i<a.num; i++)
		a[i] += b[i];
#endif
}

template<class T>
void v_list_sub(T& a, const T& b) {
#if 1
	processing::pool::run(a.num, [&a, &b] (int i) {
		a[i] -= b[i];
	}, 1000);
#else
	for (int i=0; i<a.num; i++)
		a[i] -= b[i];
#endif
}

template<class T>
void v_list_mul_single(T& a, float s) {
#if 1
	processing::pool::run(a.num, [&a, s] (int i) {
		a[i] *= s;
	}, 1000);
#else
	for (int i=0; i<a.num; i++)
		a[i] *= s;
#endif
}

void VectorField::operator+=(const VectorField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		v_list_add(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		v_list_add(v64.v, o.v64.v);
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
		v_list_sub(v32.v, o.v32.v);
	else if (type == ScalarType::Float64)
		v_list_sub(v64.v, o.v64.v);
}

VectorField VectorField::operator-(const VectorField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void VectorField::operator*=(float o) {
	if (type == ScalarType::Float32)
		v_list_mul_single(v32.v, o);
	else if (type == ScalarType::Float64)
		v_list_mul_single(v64.v, o);
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

base::tuple3<ScalarField, ScalarField, ScalarField> VectorField::split() const {
	base::tuple3<ScalarField, ScalarField, ScalarField> r = {ScalarField(grid, type, sampling_mode), ScalarField(grid, type, sampling_mode), ScalarField(grid, type, sampling_mode)};

	if (type == ScalarType::Float32) {
		processing::pool::run(v32.v.num, [this, &r] (int i) {
			r.a.v32.v[i] = v32._at(i)[0];
			r.b.v32.v[i] = v32._at(i)[1];
			r.c.v32.v[i] = v32._at(i)[2];
		}, 1000);
	} else if (type == ScalarType::Float64) {
		processing::pool::run(v64.v.num, [this, &r] (int i) {
			r.a.v64.v[i] = v64._at(i)[0];
			r.b.v64.v[i] = v64._at(i)[1];
			r.c.v64.v[i] = v64._at(i)[2];
		}, 1000);
	}
	return r;
}

ScalarField VectorField::get_component(int axis) const {;
	ScalarField s(grid, type, sampling_mode);
	if (axis < 0 or axis >= 3)
		return s;
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v32.v.num; i++) {
			s.v32.v[i] = v32._at(i)[axis];
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v64.v.num; i++) {
			s.v64.v[i] = v64._at(i)[axis];
		}
	}
	return s;
}

void VectorField::set_component(int axis, const ScalarField& s) {
	if (axis < 0 or axis >= 3)
		return;
	if (s.type != type or s.sampling_mode != sampling_mode)
		return;
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v32.v.num; i++) {
			v32._at(i)[axis] = s.v32.v[i];
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v64.v.num; i++) {
			v64._at(i)[axis] = s.v64.v[i];
		}
	}
}


VectorField VectorField::merge(const ScalarField &x, const ScalarField &y, const ScalarField &z) {
	VectorField v(x.grid, x.type, x.sampling_mode);

	if (v.type == ScalarType::Float32) {
		processing::pool::run(x.v32.v.num, [&v, &x, &y, &z] (int i) {
			v.v32._at(i)[0] = *x.v32._at(i);
			v.v32._at(i)[1] = *y.v32._at(i);
		//	v.v32._at(i)[2] = *z.v32._at(i);
		}, 1000);
	} else if (v.type == ScalarType::Float64) {
		processing::pool::run(x.v64.v.num, [&v, &x, &y, &z] (int i) {
			v.v64._at(i)[0] = *x.v64._at(i);
			v.v64._at(i)[1] = *y.v64._at(i);
			v.v64._at(i)[2] = *z.v64._at(i);
		}, 1000);
	}
	return v;
}


ScalarField VectorField::length() const {;
	ScalarField s(grid, type, sampling_mode);
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v32.v.num; i++) {
			s.v32.v[i] = ((vec3*)v32._at(i))->length();
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.v64.v.num; i++) {
			s.v64.v[i] = ((dvec3*)v64._at(i))->length();
		}
	}
	return s;
}




}
