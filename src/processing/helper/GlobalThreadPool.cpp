//
// Created by michi on 7/22/25.
//

#include "GlobalThreadPool.h"
#include <lib/math/vec3.h>
#include <lib/threads/ThreadPool.h>

#include "lib/os/msg.h"


namespace artemis::processing::pool {
ThreadPool* thread_pool = nullptr;
	void init() {
		if (!thread_pool)
			thread_pool = new ThreadPool();
	}

	void run(int n, std::function<void(int)> f, int cluster_size) {
		if (thread_pool) {
			thread_pool->run(n, f, cluster_size);
		} else {
			// dummy fall back...
			for (int i=0; i<n; i++)
				f(i);
		}
	}
	void run(const ivec3& begin, const ivec3& end, std::function<void(int,int,int)> f, int cluster_size) {
		ivec3 size = {end.i - begin.i, end.j - begin.j, end.k - begin.k};
		run(size.i * size.j * size.k, [begin, size, f] (int index) {
			int i = begin.i + (index % size.i);
			int j = begin.j + (index / size.i) % size.j;
			int k = begin.k + (index / (size.i * size.j)) % size.k;
			f(i, j, k);
		}, cluster_size);
	}
}
