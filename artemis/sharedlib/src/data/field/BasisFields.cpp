//
// Created by michi on 8/15/25.
//

#include "BasisFields.h"

#include "lib/base/map.h"

namespace artemis::data {

struct D1Basis {
	linalg::SparseMatrix<float> phi_phi;
	linalg::SparseMatrix<float> phi_d_phi;
	linalg::SparseMatrix<float> phi_dd_phi;
};

D1Basis create_basis_fields_1d(float D, int N) {
	D1Basis b;

	if (N == 0) {
		b.phi_phi.clear(1, 1);
		b.phi_phi.set(0, 0, 1);
		b.phi_d_phi.clear(1, 1);
		b.phi_d_phi.set(0, 0, 1);
		b.phi_dd_phi.clear(1, 1);
		b.phi_dd_phi.set(0, 0, 1);
		return b;
	}

	int n = N + 1;

	{
		auto& m = b.phi_phi;
		m.clear(n, n);
		const auto A = D / 6;
		// diagonal
		for (int i=0; i<N; i++)
			m.set(i,i, 4*A);
		m.set(0,0, 2*A);
		m.set(N,N, 2*A);
		// 1st parallel
		for (int i=0; i<N; i++) {
			m.set(i,i+1, A);
			m.set(i+1,i, A);
		}
	}

	{
		auto& m = b.phi_d_phi;
		m.clear(n, n);
		// diagonal
		m.set(0,0, 0.5);
		m.set(N,N, -0.5);
		// 1st parallel
		for (int i=0; i<N; i++) {
			m.set(i+1,i, 0.5);
			m.set(i,i+1, -0.5);
		}
	}

	{
		auto& m = b.phi_dd_phi;
		m.clear(n, n);
		auto A = 1.0/D;
		// diagonal
		for (int i=0; i<N; i++)
			m.set(i,i, -2*A);
		m.set(0,0, -A);
		m.set(N,N, -A);
		// 1st parallel
		for (int i=0; i<N; i++) {
			m.set(i,i+1, A);
			m.set(i+1,i, A);
		}
	}
	return b;
}

int grid_hash(const RegularGrid& g) {
	return (g.nx << 20) + (g.ny << 10) + g.nz;
}

BasisFields create_basis_fields(const RegularGrid& grid) {
	const auto bx = create_basis_fields_1d(grid.dx.length(), grid.nx);
	const auto by = create_basis_fields_1d(grid.dy.length(), grid.ny);
	const auto bz = create_basis_fields_1d(grid.dz.length(), grid.nz);

	// 2d...

	BasisFields b;
	b.grid_hash = grid_hash(grid);

	b.phi_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_phi), bz.phi_phi);
	b.phi_phi_inv = linalg::inverse(b.phi_phi);

	b.phi_dx_phi = linalg::outer_product(linalg::outer_product(bx.phi_d_phi, by.phi_phi), bz.phi_phi);
	b.phi_dy_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_d_phi), bz.phi_phi);
	b.phi_dz_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_phi), bz.phi_d_phi);

	b.phi_dx_dx_phi = linalg::outer_product(linalg::outer_product(bx.phi_dd_phi, by.phi_phi), bz.phi_phi);
	b.phi_dx_dy_phi = linalg::outer_product(linalg::outer_product(bx.phi_d_phi, by.phi_d_phi), bz.phi_phi);
	b.phi_dx_dz_phi = linalg::outer_product(linalg::outer_product(bx.phi_d_phi, by.phi_phi), bz.phi_d_phi);
	b.phi_dy_dy_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_dd_phi), bz.phi_phi);
	b.phi_dy_dz_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_d_phi), bz.phi_d_phi);
	b.phi_dz_dz_phi = linalg::outer_product(linalg::outer_product(bx.phi_phi, by.phi_phi), bz.phi_dd_phi);

	b.dx_dx = linalg::mul(b.phi_phi_inv, b.phi_dx_dx_phi.to_matrix());
	b.dx_dy = linalg::mul(b.phi_phi_inv, b.phi_dx_dy_phi.to_matrix());
	b.dx_dz = linalg::mul(b.phi_phi_inv, b.phi_dx_dz_phi.to_matrix());
	b.dy_dy = linalg::mul(b.phi_phi_inv, b.phi_dy_dy_phi.to_matrix());
	b.dy_dz = linalg::mul(b.phi_phi_inv, b.phi_dy_dz_phi.to_matrix());
	b.dz_dz = linalg::mul(b.phi_phi_inv, b.phi_dz_dz_phi.to_matrix());
	return b;
}

// crappy grid->basis database :P
const BasisFields& get_basis_fields(const RegularGrid& grid) {
	int hash = grid_hash(grid);
	static base::map<int, BasisFields*> basis_fields;
	if (!basis_fields.contains(hash)) {
		auto b = new BasisFields;
		*b = create_basis_fields(grid);
		basis_fields.set(hash, b);
	}
	return *basis_fields[hash];
}

}
