//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorField.h"
#include <processing/helper/GlobalThreadPool.h>

#include "ScalarField.h"
#include "lib/os/msg.h"

#include <cmath> // dvec3


	dvec3::dvec3(double x, double y, double z) : x(x), y(y), z(z) {}
	dvec3::dvec3() : x(0), y(0), z(0) {}
	//vec3d(const vec3d& v) : x(v.x), y(v.y), z(v.z) {}
	dvec3::dvec3(const vec3& v) : dvec3(v.x, v.y, v.z) {}
	void dvec3::operator=(const dvec3& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void dvec3::operator+=(const dvec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void dvec3::operator-=(const dvec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	void dvec3::operator*=(double s) {
		x *= s;
		y *= s;
		z *= s;
	}
	void dvec3::operator/=(double s) {
		x /= s;
		y /= s;
		z /= s;
	}
	dvec3 dvec3::operator+(const dvec3& v) const {
		return dvec3(x + v.x, y + v.y, z + v.z);
	}
	dvec3 dvec3::operator-(const dvec3& v) const {
		return dvec3(x - v.x, y - v.y, z - v.z);
	}
	dvec3 dvec3::operator*(double s) const {
		return dvec3(x * s, y * s, z * s);
	}
	dvec3 dvec3::operator/(double s) const {
		return dvec3(x / s, y / s, z / s);
	}
	vec3 dvec3::to32() const {
		return vec3((float)x, (float)y, (float)z);
	}
	double dvec3::length() const {
		return sqrt(x * x + y * y + z * z);
	}

namespace artemis::data {

VectorField::VectorField(const RegularGrid& g, ScalarType t, SamplingMode s) {
	init(g, t, 3, s);
}

VectorField::VectorField() : VectorField(RegularGrid(), ScalarType::None, SamplingMode::PerCell) {}

dvec3 VectorField::_value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return dvec3(*(vec3*)at(i, j, k));
	if (type == ScalarType::Float64)
		return *(dvec3*)at(i, j, k);
	return {0,0,0};
}

void VectorField::_set(int i, int j, int k, const dvec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)at(i, j, k) = vv.to32();
	else if (type == ScalarType::Float64)
		*(dvec3*)at(i, j, k) = vv;
}

void VectorField::set(int index, const dvec3& vv) {
	if (type == ScalarType::Float32)
		*(vec3*)at(index) = vv.to32();
	else if (type == ScalarType::Float64)
		*(dvec3*)at(index) = vv;
}

vec3 VectorField::_value32(int i, int j, int k) const {
	return _value(i, j, k).to32();
}

dvec3 VectorField::value(int index) const {
	if (type == ScalarType::Float32)
		return dvec3(*(vec3*)at(index));
	if (type == ScalarType::Float64)
		return *(dvec3*)at(index);
	return {0,0,0};
}

vec3 VectorField::value32(int index) const {
	return value(index).to32();
}

void VectorField::_set32(int i, int j, int k, const vec3& vv) {
	_set(i, j, k, dvec3(vv));
}

void VectorField::set32(int index, const vec3& vv) {
	set(index, dvec3(vv));
}

dvec3 VectorField::average() const {
	dvec3 sum = {0,0,0};
	int n = grid.count(sampling_mode);
	for (int i = 0; i<n; i++)
		sum += value(i);
	return sum / n;
}

vec3 VectorField::average32() const {
	return average().to32();
}

array_view<vec3> VectorField::as_v32() {
	return {(vec3*)data.data, n};
}

array_view<dvec3> VectorField::as_v64() {
	return {(dvec3*)data.data, n};
}

array_view<vec3> VectorField::as_v32_const() const {
	return {(vec3*)data.data, n};
}

array_view<dvec3> VectorField::as_v64_const() const {
	return {(dvec3*)data.data, n};
}

template<class T, class V>
void v_list_assign_single(T a, const V& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a.data[i] = b;
	}, 1000);
}

template<class T>
void v_list_add(T a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a.data[i] += b.data[i];
	}, 1000);
}

template<class T, class V>
void v_list_add_single(T a, const V& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a.data[i] += b;
	}, 1000);
}

template<class T>
void v_list_sub(T a, const T& b) {
	processing::pool::run(a.num, [&a, &b] (int i) {
		a.data[i] -= b.data[i];
	}, 1000);
}

template<class T>
void v_list_mul_single(T a, double s) {
	processing::pool::run(a.num, [&a, s] (int i) {
		a.data[i] *= s;
	}, 1000);
}

void VectorField::operator=(const dvec3& o) {
	if (type == ScalarType::Float32)
		v_list_assign_single(as_v32(), o.to32());
	else if (type == ScalarType::Float64)
		v_list_assign_single(as_v64(), o);
}

void VectorField::operator=(const vec3& o) {
	*this = dvec3(o);
}

void VectorField::operator+=(const VectorField& o) {
	if (o.type != type or sampling_mode != o.sampling_mode)
		return;
	if (type == ScalarType::Float32)
		v_list_add(as_v32(), o.as_v32_const());
	else if (type == ScalarType::Float64)
		v_list_add(as_v64(), o.as_v64_const());
}

void VectorField::iadd_single(const dvec3& o) {
	if (type == ScalarType::Float32)
		v_list_add_single(as_v32(), o.to32());
	else if (type == ScalarType::Float64)
		v_list_add_single(as_v64(), o);
}

void VectorField::iadd_single32(const vec3& o) {
	iadd_single(dvec3(o));
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
		v_list_sub(as_v32(), o.as_v32_const());
	else if (type == ScalarType::Float64)
		v_list_sub(as_v64(), o.as_v64_const());
}

VectorField VectorField::operator-(const VectorField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void VectorField::isub_single(const dvec3& o) {
	iadd_single({-o.x, -o.y, -o.z});
}

void VectorField::isub_single32(const vec3& o) {
	isub_single(dvec3(o));
}

void VectorField::operator*=(double o) {
	if (type == ScalarType::Float32)
		v_list_mul_single(as_v32(), o);
	else if (type == ScalarType::Float64)
		v_list_mul_single(as_v64(), o);
}

VectorField VectorField::operator*(double o) const {
	auto r = *this;
	r *= o;
	return r;
}

VectorField VectorField::componentwise_product(const VectorField& o) const {
	auto r = VectorField(grid, type, sampling_mode);
	cwise_product(r, *this, o);
	return r;
}

base::tuple3<ScalarField, ScalarField, ScalarField> VectorField::split() const {
	base::tuple3<ScalarField, ScalarField, ScalarField> r = {ScalarField(grid, type, sampling_mode), ScalarField(grid, type, sampling_mode), ScalarField(grid, type, sampling_mode)};

	processing::pool::run(n, [this, &r] (int i) {
		const auto v = value(i);
		r.a.set(i, v.x);
		r.b.set(i, v.y);
		r.c.set(i, v.z);
	}, 1000);
	return r;
}

ScalarField VectorField::get_component(int axis) const {;
	ScalarField s(grid, type, sampling_mode);
	if (axis < 0 or axis >= 3)
		return s;
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<s.n; i++) {
			((float*)s.data.data)[i] = ((float*)data.data)[i*3+axis];
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<n; i++) {
			((double*)s.data.data)[i] = ((double*)data.data)[i*3+axis];
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
		for (int i=0; i<n; i++) {
			((float*)at(i))[axis] = ((float*)s.data.data)[i];
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<n; i++) {
			((double*)at(i))[axis] = ((double*)s.data.data)[i];
		}
	}
}


VectorField VectorField::merge(const ScalarField &x, const ScalarField &y, const ScalarField &z) {
	VectorField v(x.grid, x.type, x.sampling_mode);

	processing::pool::run(x.n, [&v, &x, &y, &z] (int i) {
		v.set(i, dvec3(x.value(i), y.value(i), z.value(i)));
	}, 1000);
	return v;
}


ScalarField VectorField::length() const {;
	ScalarField s(grid, type, sampling_mode);
	if (type == ScalarType::Float32) {
		//processing::pool::run(s.v32.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<n; i++) {
			((float*)s.data.data)[i] = ((vec3*)at(i))->length();
		}
	} else if (type == ScalarType::Float64) {
		//processing::pool::run(s.v64.v.num, [this, &s, axis] (int i) {
		for (int i=0; i<n; i++) {
			((double*)s.data.data)[i] = ((dvec3*)at(i))->length();
		}
	}
	return s;
}




}
