//
// Created by michi on 7/22/25.
//

#include "GlobalThreadPool.h"


namespace artemis::processing::pool {
ThreadPool* thread_pool = nullptr;

void init() {
	if (!thread_pool)
		thread_pool = new ThreadPool();
}
}
