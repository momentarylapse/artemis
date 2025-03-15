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
				double f0 = f.value(i, j, k);
				double fx = f.value(i+1, j, k);
				double fy = f.value(i, j+1, k);
				double fz = f.value(i, j, k+1);
				v.set(i, j, k, vec3d(fx - f0, fy - f0, fz - f0));
			}

	return v;
}

data::ScalarField divergence(const data::VectorField& v) {
	data::ScalarField div(v.grid);

	for (int i=0; i<v.grid.nx-1; i++)
		for (int j=0; j<v.grid.ny-1; j++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3d v0 = v.value(i, j, k);
				double vxx = v.value(i+1, j, k).x;
				double vyy = v.value(i, j+1, k).y;
				double vzz = v.value(i, j, k+1).z;
				div.set(i, j, k, (vxx - v0.x) + (vyy - v0.y) + (vzz - v0.z));
			}

	return div;
}

data::VectorField rotation_fw(const data::VectorField& v) {
	data::VectorField rot(v.grid);

	// volume
	for (int i=0; i<v.grid.nx-1; i++)
		for (int j=0; j<v.grid.ny-1; j++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vx = v.value(i+1, j, k);
				vec3d vy = v.value(i, j+1, k);
				vec3d vz = v.value(i, j, k+1);
				rot.set(i, j, k, vec3d((vy.z - v0.z) - (vz.y - v0.y), (vz.x - v0.x) - (vx.z - v0.z), (vx.y - v0.y) - (vy.x - v0.x)));
			}
	// face x/y
	{
		int k = v.grid.nz-1;
		for (int i=0; i<v.grid.nx-1; i++)
			for (int j=0; j<v.grid.ny-1; j++) {
					vec3d v0 = v.value(i, j, k);
					vec3d vx = v.value(i+1, j, k);
					vec3d vy = v.value(i, j+1, k);
					rot.set(i, j, k, vec3d((vy.z - v0.z), - (vx.z - v0.z), (vx.y - v0.y) - (vy.x - v0.x)));
				}
	}
	// face x/z
	{
		int j = v.grid.ny-1;
		for (int i=0; i<v.grid.nx-1; i++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vx = v.value(i+1, j, k);
				vec3d vz = v.value(i, j, k+1);
				rot.set(i, j, k, vec3d(- (vz.y - v0.y), (vz.x - v0.x) - (vx.z - v0.z), (vx.y - v0.y)));
			}
	}
	// face y/z
	{
		int i = v.grid.nx-1;
		for (int j=0; j<v.grid.ny-1; j++)
			for (int k=0; k<v.grid.nz-1; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vy = v.value(i, j+1, k);
				vec3d vz = v.value(i, j, k+1);
				rot.set(i, j, k, vec3d((vy.z - v0.z) - (vz.y - v0.y), (vz.x - v0.x), - (vy.x - v0.x)));
			}
	}
	// edge x
	{
		int j = v.grid.ny-1;
		int k = v.grid.nz-1;
		for (int i=0; i<v.grid.nx-1; i++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vx = v.value(i+1, j, k);
			rot.set(i, j, k, vec3d(0, - (vx.z - v0.z), (vx.y - v0.y)));
		}
	}
	// edge y
	{
		int i = v.grid.nx-1;
		int k = v.grid.nz-1;
		for (int j=0; j<v.grid.ny-1; j++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vy = v.value(i, j+1, k);
			rot.set(i, j, k, vec3d((vy.z - v0.z), 0, - (vy.x - v0.x)));
		}
	}
	// edge z
	{
		int i = v.grid.nx-1;
		int j = v.grid.ny-1;
		for (int k=0; k<v.grid.nz-1; k++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vz = v.value(i, j, k+1);
			rot.set(i, j, k, vec3d(- (vz.y - v0.y), (vz.x - v0.x), 0));
		}
	}

	return rot;
}

data::VectorField rotation_bw(const data::VectorField& v) {
	data::VectorField rot(v.grid);

	for (int i=1; i<v.grid.nx; i++)
		for (int j=1; j<v.grid.ny; j++)
			for (int k=1; k<v.grid.nz; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vnx = v.value(i-1, j, k);
				vec3d vny = v.value(i, j-1, k);
				vec3d vnz = v.value(i, j, k-1);
				rot.set(i, j, k, vec3d((v0.z - vny.z) - (v0.y - vnz.y), (v0.x - vnz.x) - (v0.z - vnx.z), (v0.y - vnx.y) - (v0.x - vny.x)));
			}
	// face x/y
	{
		int k = 0;
		for (int i=1; i<v.grid.nx; i++)
			for (int j=1; j<v.grid.ny; j++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vnx = v.value(i-1, j, k);
				vec3d vny = v.value(i, j-1, k);
				rot.set(i, j, k, vec3d((v0.z - vny.z), - (v0.z - vnx.z), (v0.y - vnx.y) - (v0.x - vny.x)));
			}
	}
	// face x/z
	{
		int j = 0;
		for (int i=1; i<v.grid.nx; i++)
			for (int k=1; k<v.grid.nz; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vnx = v.value(i-1, j, k);
				vec3d vnz = v.value(i, j, k-1);
				rot.set(i, j, k, vec3d(- (v0.y - vnz.y), (v0.x - vnz.x) - (v0.z - vnx.z), (v0.y - vnx.y)));
			}
	}
	// face y/z
	{
		int i = 0;
		for (int j=1; j<v.grid.ny; j++)
			for (int k=1; k<v.grid.nz; k++) {
				vec3d v0 = v.value(i, j, k);
				vec3d vny = v.value(i, j-1, k);
				vec3d vnz = v.value(i, j, k-1);
				rot.set(i, j, k, vec3d((v0.z - vny.z) - (v0.y - vnz.y), (v0.x - vnz.x), - (v0.x - vny.x)));
			}
	}
	// edge x
	{
		int j = 0;
		int k = 0;
		for (int i=1; i<v.grid.nx; i++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vnx = v.value(i-1, j, k);
				rot.set(i, j, k, vec3d(0, - (v0.z - vnx.z), (v0.y - vnx.y)));
		}
	}
	// edge y
	{
		int i = 0;
		int k = 0;
		for (int j=1; j<v.grid.ny; j++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vny = v.value(i, j-1, k);
				rot.set(i, j, k, vec3d((v0.z - vny.z), 0, - (v0.x - vny.x)));
		}
	}
	// edge z
	{
		int i = 0;
		int j = 0;
		for (int k=1; k<v.grid.nz; k++) {
			vec3d v0 = v.value(i, j, k);
			vec3d vnz = v.value(i, j, k-1);
				rot.set(i, j, k, vec3d(- (v0.y - vnz.y), (v0.x - vnz.x), 0));
		}
	}

	return rot;
}

data::ScalarField laplace(const data::ScalarField& f) {
	data::ScalarField v(f.grid);

	for (int i=1; i<f.grid.nx-1; i++)
		for (int j=1; j<f.grid.ny-1; j++)
			for (int k=1; k<f.grid.nz-1; k++) {
				double f0 = f.value(i, j, k);
				double fmx = f.value(i-1, j, k);
				double fx = f.value(i+1, j, k);
				double fmy = f.value(i, j-1, k);
				double fy = f.value(i, j+1, k);
				double fmz = f.value(i, j, k-1);
				double fz = f.value(i, j, k+1);
				v.set(i, j, k, fmx+fx + fmy+fy + fmz+fz - 6*f0);
			}

	return v;
}

}

