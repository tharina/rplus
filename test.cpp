#include <array>
#include <iostream>
#include <random>
#include <stdlib.h>

using namespace std;

#include "rplus.h"

using Point = array<double, 2>;


int main(int argc, char** argv) {
    unsigned seed = 0;
    if (argc > 2)
        seed = strtol(argv[1], nullptr, 10);

   double lower_bound = -10000;
   double upper_bound = 10000;
   std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
   std::default_random_engine re;
   re.seed(seed);


   range_search::RPlusTree<Point, 64> rplus;

   vector<Point> points;
   for (int i = 0; i < 16384; i++) {
       points.push_back({{unif(re), unif(re)}});
   }

   rplus.assign(points);

   rplus.Print();

   std::vector<Point> res;
   rplus.reportRange({-10000, -10000}, {10000, 10000}, res);
   cout << res.size() << endl;
   cout << rplus.countRange({0, 0}, {10000, 10000}) << endl;

   return 0;
}
