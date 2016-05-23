#ifndef RANGE_SEARCH_NAIVE_H_
#define RANGE_SEARCH_NAIVE_H_

#include <algorithm>

#include "range_search.h"

namespace range_search {

template<class Point>
class Naive : public RangeSearch<Point> {
 public:
    void assign(const std::vector<Point>& points) override {
        dataset_ = points;
    }

    void reportRange(const Point& min, const Point& max, std::vector<Point>& result) override {
        processRange<true>(min, max, &result);
    }

    size_t countRange(const Point& min, const Point& max) override {
        return processRange<false>(min, max, nullptr);
    }

 private:
    std::vector<Point> dataset_;

    template<bool kIsReporting>
    size_t processRange(const Point& min, const Point& max, std::vector<Point>* result) {
        const auto pred = [min, max](const Point& p) {
            return min[0] <= p[0] && p[0] <= max[0]
                && min[1] <= p[1] && p[1] <= max[1];
        };
        if (kIsReporting) {
            std::copy_if(dataset_.begin(), dataset_.end(), std::back_inserter(*result), pred);
            return 0;
        } else
            return std::count_if(dataset_.begin(), dataset_.end(), pred);
    }
};

}  // namespace range_search
#endif  // RANGE_SEARCH_NAIVE_H_

