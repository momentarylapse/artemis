//
// Created by Michael Ankele on 2025-03-13.
//

#include "Calculus.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::processing {

data::VectorField gradient(const data::ScalarField& f) {
	data::VectorField v(f.grid);

	for (int i=0; i<f.grid.nx-1; i++)
		for (int j=0; j<f.grid.ny-1; j++)
			for (int k=0; k<f.grid.nz-1; k++) {
				float f0 = f.value(i, j, k);
				float fx = f.value(i+1, j, k);
				float fy = f.value(i, j+1, k);
				float fz = f.value(i, j, k+1);
				v.set(i, j, k, vec3(fx - f0, fy - f0, fz - f0));
			}

	return v;
}

data::ScalarField divergence(const data::VectorField& v) {
	data::ScalarField div(v.grid);

	for (int i=0; i<v.grid.nx-1; i++)
		for (int j=0; j<v.grid.ny-1; j++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3 v0 = v.value(i, j, k);
				float vxx = v.value(i+1, j, k).x;
				float vyy = v.value(i, j+1, k).y;
				float vzz = v.value(i, j, k+1).z;
				div.set(i, j, k, (vxx - v0.x) + (vyy - v0.y) + (vzz - v0.z));
			}

	return div;
}

data::VectorField rotation_fw(const data::VectorField& v) {
	data::VectorField rot(v.grid);

	for (int i=0; i<v.grid.nx-1; i++)
		for (int j=0; j<v.grid.ny-1; j++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3 v0 = v.value(i, j, k);
				vec3 vx = v.value(i+1, j, k);
				vec3 vy = v.value(i, j+1, k);
				vec3 vz = v.value(i, j, k+1);
				rot.set(i, j, k, vec3((vy.z - v0.z) - (vz.y - v0.y), (vz.x - v0.x) - (vx.z - v0.z), (vx.y - v0.y) - (vy.z - v0.z)));
			}

	return rot;
}

data::VectorField rotation_bw(const data::VectorField& v) {
	data::VectorField rot(v.grid);

	for (int i=1; i<v.grid.nx; i++)
		for (int j=1; j<v.grid.ny; j++)
			for (int k=1; k<v.grid.nz; k++) {
				vec3 v0 = v.value(i, j, k);
				vec3 vnx = v.value(i-1, j, k);
				vec3 vny = v.value(i, j-1, k);
				vec3 vnz = v.value(i, j, k-1);
				rot.set(i, j, k, vec3((v0.z - vny.z) - (v0.y - vnz.y), (v0.x - vnz.x) - (v0.z - vnx.z), (v0.y - vnx.y) - (v0.z - vny.z)));
			}

	return rot;
}

data::ScalarField laplace(const data::ScalarField& f) {
	data::ScalarField v(f.grid);

	for (int i=1; i<f.grid.nx-1; i++)
		for (int j=1; j<f.grid.ny-1; j++)
			for (int k=1; k<f.grid.nz-1; k++) {
				float f0 = f.value(i, j, k);
				float fmx = f.value(i-1, j, k);
				float fx = f.value(i+1, j, k);
				float fmy = f.value(i, j-1, k);
				float fy = f.value(i, j+1, k);
				float fmz = f.value(i, j, k-1);
				float fz = f.value(i, j, k+1);
				v.set(i, j, k, fmx+fx + fmy+fy + fmz+fz - 6*f0);
			}

	return v;
}

}

