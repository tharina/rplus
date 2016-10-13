#include <array>
#include <iostream>
#include <random>

using namespace std;

#include "rplus.h"

using Point = array<double, 2>;


int main()
{
   double lower_bound = 0;
   double upper_bound = 10000;
   std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
   std::default_random_engine re;


   range_search::RPlusTree<Point> rplus;

   vector<Point> points;
   for (int i = 0; i < 100; i++) {
       points.push_back({{unif(re), unif(re)}});
   }

   rplus.assign(points);

   return 0;
}
