//
// Created by michi on 8/14/25.
//

#pragma once
#include <lib/base/base.h>
#include <armadillo>

namespace linalg {

template<class T>
struct Matrix {
	arma::Mat<T> e;
	int rows = 0;
	int cols = 0;
	Matrix() {}
	Matrix(int _rows, int _cols) {
		rows = _rows;
		cols = _cols;
		e.resize(rows, cols);
	}
	void clear(int _rows, int _cols) {
		*this = Matrix(_rows, _cols);
	}
	T& operator()(int i, int j) {
		return e(i, j);
	}
	const T& operator()(int i, int j) const {
		return e(i, j);
	}
	const T* get(int i, int j) {
		return &e(i, j);
	}
};

template<class T>
Matrix<T> zeros(int rows, int cols) {
	return Matrix<T>(rows, cols);
}

template<class T>
Matrix<T> eye(int n) {
	Matrix<T> mat(n, n);
	for (int i=0;i<n;i++)
		mat(i,i) = 1;
	return mat;
}

template<class T>
Array<T> mul(const Matrix<T>& A, const Array<T>& x) {
	Array<T> y;
	y.resize(A.rows);
	arma::Col<T> yy(&y[0], y.num, false);
	yy = A.e * arma::Col<T>(&x[0], x.num); // seems to write into memory... (^_^)'
	return y;
}

template<class T>
Matrix<T> mul(const Matrix<T>& A, const Matrix<T>& B) {
	Matrix<T> C(A.rows, B.cols);
	C.e = A.e * B.e;
	return C;
}

template<class T>
Array<T> solve(const Matrix<T>& M, const Array<T>& b) {
	Array<T> x;
	x.resize(M.cols);
	arma::Col<T> xx(&x[0], x.num, false);
	if (!arma::solve(xx, M.e, arma::Col<T>(&b[0], b.num, false))) {}
	return x;
}

template<class T>
Matrix<T> inverse(const Matrix<T>& A) {
	Matrix<T> C(A.cols, A.rows);
	C.e = A.e.i();
	return C;
}




template<class T>
struct SparseMatrix {
	arma::SpMat<T> e;
	int rows = 0;
	int cols = 0;
	SparseMatrix() {}
	SparseMatrix(int _rows, int _cols) {
		rows = _rows;
		cols = _cols;
		e.resize(rows, cols);
	}
	void clear(int _rows, int _cols) {
		*this = SparseMatrix(_rows, _cols);
	}
	T operator()(int i, int j) const {
		return e(i, j);
	}
	T get(int i, int j) {
		return e(i, j);
	}
	void set(int i, int j, T v) {
		e(i, j) = v;
	}
	Matrix<T> to_matrix() const {
		Matrix<T> mat(rows, cols);
		mat.e = e;
		return mat;
	}
};

template<class T>
Array<T> mul(const SparseMatrix<T>& M, const Array<T>& x) {
	Array<T> y;
	y.resize(M.rows);
	arma::Col<T> yy(&y[0], y.num, false);
	yy = M.e * arma::Col<T>(&x[0], x.num); // seems to write into memory... (^_^)'
	return y;
}

template<class T>
Matrix<T> inverse(const SparseMatrix<T>& A) {
	Matrix<T> C(A.cols, A.rows);
	arma::Mat<T> AA;
	AA = A.e;
	C.e = AA.i(); // direct SpMat.i() not supported...
	return C;
}

// A: inner, B: outer
template<class T>
SparseMatrix<T> outer_product(const SparseMatrix<T>& A, const SparseMatrix<T>& B) {
	SparseMatrix<T> C(A.rows * B.rows, A.cols * B.cols);
	C.e = arma::kron(B.e, A.e);
	return C;
}


// A: inner, B: outer
/*	func outer_product(A: SparseRowMatrix, B: SparseRowMatrix) -> SparseRowMatrix
		var r: Ret
		r.clear(A.m*B.m, A.n*B.n)

		for i=>rowA in A.rows
			for j=>rowB in B.rows
				for eA in rowA
					for eB in rowB
						r.set(j*A.m+i, eB[0]*A.m+eA[0], eA[1]*eB[1])
		return r
*/

} // linalg
