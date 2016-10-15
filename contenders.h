#ifndef CONTENDERS_H_
#define CONTENDERS_H_

#include "naive.h"
#include "rplus.h"

#elif defined ADD_CONTENDERS

experiments.addContender("Naive", []() { return new Naive<Point>; });
experiments.addContender("R+Tree8", []() { return new RPlusTree<Point, 8>; });
experiments.addContender("R+Tree16", []() { return new RPlusTree<Point, 16>; });
experiments.addContender("R+Tree32", []() { return new RPlusTree<Point, 32>; });
experiments.addContender("R+Tree64", []() { return new RPlusTree<Point, 64>; });
experiments.addContender("R+Tree128", []() { return new RPlusTree<Point, 128>; });
experiments.addContender("R+Tree256", []() { return new RPlusTree<Point, 256>; });
experiments.addContender("R+Tree512", []() { return new RPlusTree<Point, 512>; });
experiments.addContender("R+Tree1024", []() { return new RPlusTree<Point, 1024>; });

#endif

