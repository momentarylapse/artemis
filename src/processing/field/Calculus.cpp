//
// Created by Michael Ankele on 2025-03-13.
//

#include "Calculus.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <data/field/BasisFields.h>
#include "../helper/GlobalThreadPool.h"

namespace artemis::processing {

data::VectorField gradient(const data::ScalarField& f) {
	data::VectorField v(f.grid, f.type, f.sampling_mode);

	pool::run({0,0,0}, {f.grid.nx-1,f.grid.ny-1,f.grid.nz-1}, [&f, &v] (int i, int j, int k) {
		double f0 = f.value(i, j, k);
		double fx = f.value(i+1, j, k);
		double fy = f.value(i, j+1, k);
		double fz = f.value(i, j, k+1);
		v.set(i, j, k, dvec3(fx - f0, fy - f0, fz - f0));
	}, 200);

	return v;
}

data::ScalarField divergence(const data::VectorField& v) {
	data::ScalarField div(v.grid, v.type, v.sampling_mode);

	pool::run({0,0,0}, {v.grid.nx-1,v.grid.ny-1,v.grid.nz-1}, [&div, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		double vxx = v.value(i+1, j, k).x;
		double vyy = v.value(i, j+1, k).y;
		double vzz = v.value(i, j, k+1).z;
		div.set(i, j, k, (vxx - v0.x) + (vyy - v0.y) + (vzz - v0.z));
	}, 200);

	return div;
}

data::VectorField rotation_fw(const data::VectorField& v) {
	data::VectorField rot(v.grid, v.type, v.sampling_mode);

	// volume
	pool::run({0,0,0}, {v.grid.nx-1, v.grid.ny-1, v.grid.nz-1}, [&v, &rot] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vx = v.value(i+1, j, k);
		dvec3 vy = v.value(i, j+1, k);
		dvec3 vz = v.value(i, j, k+1);
		rot.set(i, j, k, dvec3((vy.z - v0.z) - (vz.y - v0.y), (vz.x - v0.x) - (vx.z - v0.z), (vx.y - v0.y) - (vy.x - v0.x)));
	}, 200);
	// face x/y
	pool::run({0,0,v.grid.nz-1}, {v.grid.nx-1,v.grid.ny-1,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vx = v.value(i+1, j, k);
		dvec3 vy = v.value(i, j+1, k);
		rot.set(i, j, k, dvec3((vy.z - v0.z), - (vx.z - v0.z), (vx.y - v0.y) - (vy.x - v0.x)));
	}, 200);
	// face x/z
	pool::run({0,v.grid.ny-1,0}, {v.grid.nx-1,v.grid.ny,v.grid.nz-1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vx = v.value(i+1, j, k);
		dvec3 vz = v.value(i, j, k+1);
		rot.set(i, j, k, dvec3(- (vz.y - v0.y), (vz.x - v0.x) - (vx.z - v0.z), (vx.y - v0.y)));
	}, 200);
	// face y/z
	pool::run({v.grid.nx-1,0,0}, {v.grid.nx,v.grid.ny-1,v.grid.nz-1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vy = v.value(i, j+1, k);
		dvec3 vz = v.value(i, j, k+1);
		rot.set(i, j, k, dvec3((vy.z - v0.z) - (vz.y - v0.y), (vz.x - v0.x), - (vy.x - v0.x)));
	}, 200);
	// edge x
	pool::run({0,v.grid.ny-1,v.grid.nz-1}, {v.grid.nx-1,v.grid.ny,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vx = v.value(i+1, j, k);
		rot.set(i, j, k, dvec3(0, - (vx.z - v0.z), (vx.y - v0.y)));
	}, 200);
	// edge y
	pool::run({v.grid.nx-1,0,v.grid.nz-1}, {v.grid.nx,v.grid.ny-1,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vy = v.value(i, j+1, k);
		rot.set(i, j, k, dvec3((vy.z - v0.z), 0, - (vy.x - v0.x)));
	}, 200);
	// edge z
	pool::run({v.grid.nx-1,v.grid.ny-1,0}, {v.grid.nx,v.grid.ny,v.grid.nz-1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vz = v.value(i, j, k+1);
		rot.set(i, j, k, dvec3(- (vz.y - v0.y), (vz.x - v0.x), 0));
	}, 200);

	return rot;
}

data::VectorField rotation_bw(const data::VectorField& v) {
	data::VectorField rot(v.grid, v.type, v.sampling_mode);

	pool::run({1,1,1}, {v.grid.nx, v.grid.ny, v.grid.nz}, [&v, &rot] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vnx = v.value(i-1, j, k);
		dvec3 vny = v.value(i, j-1, k);
		dvec3 vnz = v.value(i, j, k-1);
		rot.set(i, j, k, dvec3((v0.z - vny.z) - (v0.y - vnz.y), (v0.x - vnz.x) - (v0.z - vnx.z), (v0.y - vnx.y) - (v0.x - vny.x)));
	}, 200);
	// face x/y
	pool::run({1,1,0}, {v.grid.nx,v.grid.ny,1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vnx = v.value(i-1, j, k);
		dvec3 vny = v.value(i, j-1, k);
		rot.set(i, j, k, dvec3((v0.z - vny.z), - (v0.z - vnx.z), (v0.y - vnx.y) - (v0.x - vny.x)));
	}, 200);
	// face x/z
	pool::run({1,0,1}, {v.grid.nx,1,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vnx = v.value(i-1, j, k);
		dvec3 vnz = v.value(i, j, k-1);
		rot.set(i, j, k, dvec3(- (v0.y - vnz.y), (v0.x - vnz.x) - (v0.z - vnx.z), (v0.y - vnx.y)));
	}, 200);
	// face y/z
	pool::run({0,1,1}, {1,v.grid.ny,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vny = v.value(i, j-1, k);
		dvec3 vnz = v.value(i, j, k-1);
		rot.set(i, j, k, dvec3((v0.z - vny.z) - (v0.y - vnz.y), (v0.x - vnz.x), - (v0.x - vny.x)));
	}, 200);
	// edge x
	pool::run({1,0,0}, {v.grid.nx,1,1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vnx = v.value(i-1, j, k);
			rot.set(i, j, k, dvec3(0, - (v0.z - vnx.z), (v0.y - vnx.y)));
	}, 200);
	// edge y
	pool::run({0,1,0}, {1,v.grid.ny,1}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vny = v.value(i, j-1, k);
		rot.set(i, j, k, dvec3((v0.z - vny.z), 0, - (v0.x - vny.x)));
	}, 200);
	// edge z
	pool::run({0,0,1}, {1,1,v.grid.nz}, [&rot, &v] (int i, int j, int k) {
		dvec3 v0 = v.value(i, j, k);
		dvec3 vnz = v.value(i, j, k-1);
		rot.set(i, j, k, dvec3(- (v0.y - vnz.y), (v0.x - vnz.x), 0));
	}, 200);

	return rot;
}

template<class T>
void t_laplace(const data::RegularGrid& grid, const T& f, T& f_out) {
	pool::run({1,1,1}, {grid.nx-1,grid.ny-1,grid.nz-1}, [&grid, &f, &f_out] (int i, int j, int k) {
		const auto f0 = f.v[grid.cell_index(i, j, k)];
		const auto fmx = f.v[grid.cell_index(i-1, j, k)];
		const auto fx = f.v[grid.cell_index(i+1, j, k)];
		const auto fmy = f.v[grid.cell_index(i, j-1, k)];
		const auto fy = f.v[grid.cell_index(i, j+1, k)];
		const auto fmz = f.v[grid.cell_index(i, j, k-1)];
		const auto fz = f.v[grid.cell_index(i, j, k+1)];
		f_out.v[grid.cell_index(i, j, k)] = fmx+fx + fmy+fy + fmz+fz - 6*f0;
	}, 200);
}

data::ScalarField laplace(const data::ScalarField& f) {
	data::ScalarField out(f.grid, f.type, f.sampling_mode);
	if (f.type == data::ScalarType::Float32)
		t_laplace(f.grid, f.v32, out.v32);
	else if (f.type == data::ScalarType::Float64)
		t_laplace(f.grid, f.v64, out.v64);
	return out;
}

data::ScalarField hessian_x(const data::ScalarField& f, int i, int j) {
	data::ScalarField out(f.grid, f.type, f.sampling_mode);
	if (f.type == data::ScalarType::Float32) {
		auto& b = data::get_basis_fields(f.grid);
		if (i == 0 and j == 0)
			//out.v32.v = linalg::mul(b.phi_phi_inv, linalg::mul(b.phi_dx_dx_phi, f.v32.v));
				out.v32.v = linalg::mul(b.dx_dx, f.v32.v);
		if (i == 1 and j == 1)
				out.v32.v = linalg::mul(b.dy_dy, f.v32.v);
		if (i == 2 and j == 2)
			out.v32.v = linalg::mul(b.dz_dz, f.v32.v);
		if ((i == 0 and j == 1) or (i == 1 and j == 0))
			out.v32.v = linalg::mul(b.dx_dy, f.v32.v);
		if ((i == 0 and j == 2) or (i == 2 and j == 0))
			out.v32.v = linalg::mul(b.dx_dz, f.v32.v);
		if ((i == 1 and j == 2) or (i == 2 and j == 1))
			out.v32.v = linalg::mul(b.dy_dz, f.v32.v);

	}
	return out;
}

}

