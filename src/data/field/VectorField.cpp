//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorField.h"

namespace artemis::data {

VectorField::VectorField(const RegularGrid& g) {
	grid = g;
	v.resize(grid.nx * grid.ny * grid.nz);
}

VectorField::VectorField() : VectorField(RegularGrid()) {}

vec3d VectorField::value(int i, int j, int k) const {
	return v[i + j * grid.nx + k * grid.nx * grid.ny];
}

void VectorField::set(int i, int j, int k, const vec3d& vv) {
	v[i + j * grid.nx + k * grid.nx * grid.ny] = vv;
}

vec3 VectorField::value32(int i, int j, int k) const {
	return v[i + j * grid.nx + k * grid.nx * grid.ny].to32();
}

void VectorField::set32(int i, int j, int k, const vec3& vv) {
	v[i + j * grid.nx + k * grid.nx * grid.ny] = vec3d(vv);
}

void VectorField::operator+=(const VectorField& o) {
	for (int i=0; i<v.num; i++)
		v[i] += o.v[i];
}

VectorField VectorField::operator+(const VectorField& o) const {
	auto r = *this;
	r += o;
	return r;
}

void VectorField::operator-=(const VectorField& o) {
	for (int i=0; i<v.num; i++)
		v[i] -= o.v[i];
}

VectorField VectorField::operator-(const VectorField& o) const {
	auto r = *this;
	r -= o;
	return r;
}

void VectorField::operator*=(float o) {
	for (auto& x: v)
		x *= o;
}

VectorField VectorField::operator*(float o) const {
	auto r = *this;
	r *= o;
	return r;
}

VectorField VectorField::componentwise_product(const VectorField& o) const {
	auto times = [] (const vec3d& a, const vec3d& b) {
		return vec3d(a.x * b.x, a.y * b.y, a.z * b.z);
	};
	auto r = VectorField(grid);
	for (int i=0; i<v.num; i++)
		r.v[i] = times(v[i], o.v[i]);
	return r;
}



}
