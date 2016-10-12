#ifndef CONTENDERS_H_
#define CONTENDERS_H_

#include "naive.h"
#include "rplus.h"

#elif defined ADD_CONTENDERS

experiments.addContender("Naive", []() { return new Naive<Point>; });
experiments.addContender("R+Tree", []() { return new RPlusTree<Point>; });

#endif

