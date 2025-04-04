//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"
#include <lib/math/vec3.h>

struct dvec3 {
	double x, y, z;
	dvec3(double x, double y, double z) : x(x), y(y), z(z) {}
	dvec3() : x(0), y(0), z(0) {}
	//vec3d(const vec3d& v) : x(v.x), y(v.y), z(v.z) {}
	explicit dvec3(const vec3& v) : dvec3(v.x, v.y, v.z) {}
	void operator=(const dvec3& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void operator+=(const dvec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator-=(const dvec3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	void operator*=(double s) {
		x *= s;
		y *= s;
		z *= s;
	}
	void operator/=(double s) {
		x /= s;
		y /= s;
		z /= s;
	}
	dvec3 operator+(const dvec3& v) const {
		return dvec3(x + v.x, y + v.y, z + v.z);
	}
	dvec3 operator-(const dvec3& v) const {
		return dvec3(x - v.x, y - v.y, z - v.z);
	}
	dvec3 operator*(double s) const {
		return dvec3(x * s, y * s, z * s);
	}
	dvec3 operator/(double s) const {
		return dvec3(x / s, y / s, z / s);
	}
	vec3 to32() const {
		return vec3((float)x, (float)y, (float)z);
	}
};

namespace artemis::data {

struct VectorField {
	VectorField();
	explicit VectorField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode);

	ScalarType type;
	RegularGrid grid;
	SamplingMode sampling_mode;
	SampledData<vec3, float> v32;
	SampledData<dvec3, double> v64;

	dvec3 value(int i, int j, int k) const;
	void set(int i, int j, int k, const dvec3& v);
	vec3 value32(int i, int j, int k) const;
	void set32(int i, int j, int k, const vec3& v);

	void operator+=(const VectorField& o);
	VectorField operator+(const VectorField& o) const;
	void operator-=(const VectorField& o);
	VectorField operator-(const VectorField& o) const;
	void operator*=(float o);
	VectorField operator*(float o) const;

	VectorField componentwise_product(const VectorField& o) const;
};

}


