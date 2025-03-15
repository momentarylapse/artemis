//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "../grid/RegularGrid.h"
#include <lib/math/vec3.h>

struct vec3d {
	double x, y, z;
	vec3d(double x, double y, double z) : x(x), y(y), z(z) {}
	vec3d() : x(0), y(0), z(0) {}
	//vec3d(const vec3d& v) : x(v.x), y(v.y), z(v.z) {}
	explicit vec3d(const vec3& v) : vec3d(v.x, v.y, v.z) {}
	void operator=(const vec3d& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	void operator+=(const vec3d& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator-=(const vec3d& v) {
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
	vec3d operator+(const vec3d& v) const {
		return vec3d(x + v.x, y + v.y, z + v.z);
	}
	vec3d operator-(const vec3d& v) const {
		return vec3d(x - v.x, y - v.y, z - v.z);
	}
	vec3d operator*(double s) const {
		return vec3d(x * s, y * s, z * s);
	}
	vec3d operator/(double s) const {
		return vec3d(x / s, y / s, z / s);
	}
	vec3 to32() const {
		return vec3((float)x, (float)y, (float)z);
	}
};

namespace artemis::data {

struct VectorField {
	VectorField();
	explicit VectorField(const RegularGrid& grid);

	RegularGrid grid;
	Array<vec3d> v;

	vec3d value(int i, int j, int k) const;
	void set(int i, int j, int k, const vec3d& v);
	vec3 value32(int i, int j, int k) const;
	void set32(int i, int j, int k, const vec3& v);

	void operator+=(const VectorField& o);
	VectorField operator+(const VectorField& o) const;
	void operator-=(const VectorField& o);
	VectorField operator-(const VectorField& o) const;
	void operator*=(float o);
	VectorField operator*(float o) const;
};

}


