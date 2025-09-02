//
// Created by Michael Ankele on 2025-03-11.
//

#include "IsoSurface.h"
#include <lib/mesh/PolygonMesh.h>
#include <data/field/ScalarField.h>
#include <lib/base/sort.h>
#include <lib/math/mat4.h>
#include <cmath>

namespace artemis::processing {


void iso_cell(const artemis::data::ScalarField& f, PolygonMesh& mesh, int i, int j, int k, float t0) {
	float v[8];
	v[0] = f._value32(i, j, k) - t0;
	v[1] = f._value32(i, j, k+1) - t0;
	v[2] = f._value32(i, j+1, k) - t0;
	v[3] = f._value32(i, j+1, k+1) - t0;
	v[4] = f._value32(i+1, j, k) - t0;
	v[5] = f._value32(i+1, j, k+1) - t0;
	v[6] = f._value32(i+1, j+1, k) - t0;
	v[7] = f._value32(i+1, j+1, k+1) - t0;
	bool s[8];
	for (int n=0; n<8; n++)
		s[n] = v[n] > 0;

	bool any_diffs = false;
	for (int n=1; n<8; n++)
		if (s[n] != s[0])
			any_diffs = true;
	if (!any_diffs)
		return;

	auto inter = [&] (int n0, int n1) {
		float t = fabs(v[n0]) / fabs(v[n1] - v[n0]);
		vec3 p0 = f.grid.index_to_pos(i + ((n0 >> 2) & 1), j + ((n0 >> 1) & 1), k + (n0 & 1));
		vec3 p1 = f.grid.index_to_pos(i + ((n1 >> 2) & 1), j + ((n1 >> 1) & 1), k + (n1 & 1));
		return p0 + (p1 - p0) * t;
	};

	// triangles around single corners
	auto corner = [&] (int n0) {
		int n1 = n0 ^ 1;
		int n2 = n0 ^ 2;
		int n3 = n0 ^ 4;
		if (s[n0] != s[n1] and s[n0] != s[n2] and s[n0] != s[n3]) {
			int nv0 = mesh.vertices.num;
			mesh.add_vertex(inter(n0, n1));
			mesh.add_vertex(inter(n0, n2));
			mesh.add_vertex(inter(n0, n2));
			mesh.add_polygon_auto_texture({nv0, nv0+1, nv0+2});
		}
	};
	for (int n=0; n<8; n++)
		corner(n);

	// quad through center
	auto center_quad = [&] (int axis) {
		// TODO
		if (s[0] != s[1] and s[2] != s[3] and s[4] != s[5] and s[6] != s[7] and s[0] == s[1] == s[2] == s[3]) {
			int nv0 = mesh.vertices.num;
			mesh.add_vertex(inter(0, 1));
			mesh.add_vertex(inter(2, 3));
			mesh.add_vertex(inter(4, 5));
			mesh.add_vertex(inter(6, 7));
			mesh.add_polygon_auto_texture({nv0, nv0+1, nv0+3, nv0+2});
		}
	};
	//for (int axis=1; axis<=4; axis<<=1)
	//	center_quad(axis);
	center_quad(-1);

	// argh...nope  m(-_-)m
}

// lazy-man's method :D
void iso_cell_approx(const artemis::data::ScalarField& f, PolygonMesh& mesh, int i, int j, int k, float t0) {
	float v[8];
	v[0] = f._value32(i, j, k) - t0;
	v[1] = f._value32(i, j, k+1) - t0;
	v[2] = f._value32(i, j+1, k) - t0;
	v[3] = f._value32(i, j+1, k+1) - t0;
	v[4] = f._value32(i+1, j, k) - t0;
	v[5] = f._value32(i+1, j, k+1) - t0;
	v[6] = f._value32(i+1, j+1, k) - t0;
	v[7] = f._value32(i+1, j+1, k+1) - t0;
	bool s[8];
	for (int n=0; n<8; n++)
		s[n] = v[n] > 0;

	bool any_diffs = false;
	for (int n=1; n<8; n++)
		if (s[n] != s[0])
			any_diffs = true;
	if (!any_diffs)
		return;

	auto inter = [&] (int n0, int n1) {
		float t = fabs(v[n0]) / fabs(v[n1] - v[n0]);
		vec3 p0 = f.grid.index_to_pos(i + ((n0 >> 2) & 1), j + ((n0 >> 1) & 1), k + (n0 & 1));
		vec3 p1 = f.grid.index_to_pos(i + ((n1 >> 2) & 1), j + ((n1 >> 1) & 1), k + (n1 & 1));
		return p0 + (p1 - p0) * t;
	};

	Array<vec3> points;
	// check all edges
	for (int n0=0; n0<8; n0++)
		for (int axis=1; axis<=4; axis<<=1) {
			int n1 = n0 ^ axis;
			if (n1 > n0 and s[n0] != s[n1])
				points.add(inter(n0, n1));
		}
	if (points.num < 3)
		return;

	// project onto plane, orthogonal to gradient
	vec3 grad = vec3(v[4]+v[5]+v[6]+v[7] - (v[0]+v[1]+v[2]+v[3]),
				v[2]+v[3]+v[6]+v[7] - (v[0]+v[1]+v[4]+v[5]),
				v[1]+v[3]+v[5]+v[7] - (v[0]+v[2]+v[4]+v[6])).normalized();
	vec3 e2 = grad.ortho();
	vec3 e3 = vec3::cross(grad, e2);
	vec3 cell_center = (f.grid.index_to_pos(i, j, k) + f.grid.index_to_pos(i+1, j+1, k+1)) / 2;

	// sort by angle around cell_center
	auto phi = [&] (const vec3 p) {
		return atan2(vec3::dot(p - cell_center, e3), vec3::dot(p - cell_center, e2));
	};
	points = base::sorted(points, [&] (const vec3& a, const vec3& b) {
		return phi(a) <= phi(b);
	});

	int nv0 = mesh.vertices.num;
	for (const vec3& p: points)
		mesh.add_vertex(p);
	Array<int> indices;
	for (int n=0; n<points.num; n++)
		indices.add(nv0 + n);
	mesh.add_polygon_auto_texture(indices); // range(nv, nv+points.num)
}

PolygonMesh iso_surface(const data::ScalarField& f, float t0) {
	PolygonMesh mesh;
	if (f.sampling_mode == artemis::data::SamplingMode::PerCell) {
		for (int i=0; i<f.grid.nx-1; i++)
			for (int j=0; j<f.grid.ny-1; j++)
				for (int k=0; k<f.grid.nz-1; k++)
					iso_cell_approx(f, mesh, i, j, k, t0);
		mesh = mesh.transform(mat4::translation(f.grid.cell_center(0,0,0) - f.grid.offset));
	} else if (f.sampling_mode == artemis::data::SamplingMode::PerVertex) {
		for (int i=0; i<f.grid.nx; i++)
			for (int j=0; j<f.grid.ny; j++)
				for (int k=0; k<f.grid.nz; k++)
					iso_cell_approx(f, mesh, i, j, k, t0);
	}
	return mesh;
}


}
