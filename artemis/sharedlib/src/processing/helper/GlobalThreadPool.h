//
// Created by michi on 7/22/25.
//

#ifndef GLOBALTHREADPOOL_H
#define GLOBALTHREADPOOL_H

#include <lib/threads/ThreadPool.h>
#include <lib/math/vec3.h>

struct ivec3;

namespace artemis::processing::pool {

extern ThreadPool* thread_pool;

void init();

template<class F> // int -> void
void run(int n, F f, int cluster_size) {
	if (thread_pool) {
		thread_pool->run(n, f, cluster_size);
	} else {
		// dummy fall back...
		for (int i=0; i<n; i++)
			f(i);
	}
}

//void run(const ivec3& begin, const ivec3& end, std::function<void(int,int,int)> f, int cluster_size=1);

template<class F> // (int,int,int) -> void
void run(const ivec3& begin, const ivec3& end, F f, int cluster_size) {
	ivec3 size = {end.i - begin.i, end.j - begin.j, end.k - begin.k};
	run(size.i * size.j * size.k, [begin, size, f] (int index) {
		int i = begin.i + (index % size.i);
		int j = begin.j + (index / size.i) % size.j;
		int k = begin.k + (index / (size.i * size.j)) % size.k;
		f(i, j, k);
	}, cluster_size);
}
}

#endif //GLOBALTHREADPOOL_H
