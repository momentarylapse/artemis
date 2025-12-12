//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"
#include <lib/base/tuple.h>
#include <lib/math/vec3.h>

struct dvec3 {
	double x, y, z;
	dvec3(double x, double y, double z);
	dvec3();
	//vec3d(const vec3d& v) : x(v.x), y(v.y), z(v.z) {}
	explicit dvec3(const vec3& v);
	void operator=(const dvec3& v);
	void operator+=(const dvec3& v);
	void operator-=(const dvec3& v);
	void operator*=(double s);
	void operator/=(double s);
	dvec3 operator+(const dvec3& v) const;
	dvec3 operator-(const dvec3& v) const;
	dvec3 operator*(double s) const;
	dvec3 operator/(double s) const;
	vec3 to32() const;
	double length() const;
};

namespace artemis::data {

struct ScalarField;

struct VectorField {
	VectorField();
	explicit VectorField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode);

	ScalarType type;
	RegularGrid grid;
	SamplingMode sampling_mode;
	SampledData<float> v32;
	SampledData<double> v64;

	void set(int index, const dvec3& v);
	void set32(int index, const vec3& v);
	dvec3 value(int index) const;
	vec3 value32(int index) const;
	dvec3 _value(int i, int j, int k) const;
	void _set(int i, int j, int k, const dvec3& v);
	vec3 _value32(int i, int j, int k) const;
	void _set32(int i, int j, int k, const vec3& v);

	dvec3 average() const;
	vec3 average32() const;

	void operator=(const dvec3& o);
	void operator=(const vec3& o);
	void operator+=(const VectorField& o);
//	void operator+=(const dvec3& o);
	void iadd_single(const dvec3& o);
	void iadd_single32(const vec3& o);
	VectorField operator+(const VectorField& o) const;
	void operator-=(const VectorField& o);
//	void operator-=(const dvec3& o);
	void isub_single(const dvec3& o);
	void isub_single32(const vec3& o);
	VectorField operator-(const VectorField& o) const;
	void operator*=(double o);
	VectorField operator*(double o) const;

	VectorField componentwise_product(const VectorField& o) const;
	//void split(ScalarField& x, ScalarField& y, ScalarField& z) const;
	base::tuple3<ScalarField, ScalarField, ScalarField> split() const;
	ScalarField get_component(int i) const;
	ScalarField length() const;
	void set_component(int i, const ScalarField& s);
	static VectorField merge(const ScalarField& x, const ScalarField& y, const ScalarField& z);
};

}


