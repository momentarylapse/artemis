//
// Created by michi on 7/22/25.
//

#ifndef GLOBALTHREADPOOL_H
#define GLOBALTHREADPOOL_H

#include <functional>

struct ivec3;

namespace artemis::processing::pool {
void init();
void run(int n, std::function<void(int)> f, int cluster_size=1);
void run(const ivec3& begin, const ivec3& end, std::function<void(int,int,int)> f, int cluster_size=1);
}

#endif //GLOBALTHREADPOOL_H
