#ifndef FRAMEWORK_BENCHMARK_H_
#define FRAMEWORK_BENCHMARK_H_

#include <algorithm>
#include <iostream>
#include <ostream>

#include "../range_search.h"

namespace framework {

template<class Datastructure>
class Benchmark {
 public:
    virtual ~Benchmark() = default;

    virtual void runPreprocessing(Datastructure&) = 0;
    virtual void runQueries(Datastructure&) = 0;
    virtual bool compare(Datastructure& lhs, Datastructure& rhs) = 0;
    virtual std::ostream& result(std::ostream&) const = 0;
};

template<class Point>
class RangeSearchQueries : public Benchmark<range_search::RangeSearch<Point>> {
 public:
    RangeSearchQueries(std::vector<Point> dataset,
            std::vector<std::pair<Point, Point>> queries,
            bool reporting_query = false,
            std::string params = "")
        : dataset_(std::move(dataset))
        , queries_(std::move(queries))
        , params_(std::move(params))
        , reporting_query_(reporting_query)
    {
        if (reporting_query_)
            result_.reserve(dataset_.size());
    }

    void runPreprocessing(range_search::RangeSearch<Point>& rs) override {
        rs.assign(dataset_);
    }

    void runQueries(range_search::RangeSearch<Point>& rs) override {
        if (reporting_query_)
            for (const auto& q : queries_) {
                rs.reportRange(q.first, q.second, result_);
                result_.clear();
            }
        else for (const auto& q : queries_)
            rs.countRange(q.first, q.second);
    }

    std::ostream& result(std::ostream& str) const override {
        return str << "\tsize=" << dataset_.size()
            << "\tqueries=" << queries_.size()
            << "\treporting=" << reporting_query_
            << params_;
    }

    bool compare(range_search::RangeSearch<Point>& lhs, range_search::RangeSearch<Point>& rhs) override {
        bool mismatch = false;
        std::vector<Point> result2;
        std::vector<Point> diff;
        for (const auto& q : queries_) {
            lhs.reportRange(q.first, q.second, result_);
            rhs.reportRange(q.first, q.second, result2);
            std::sort(result_.begin(), result_.end());
            std::sort(result2.begin(), result2.end());
            if (result_ != result2) {
                mismatch = true;
                std::cerr << "Mismatch in query [(" << q.first[0] << ',' << q.first[1]
                    << "), (" << q.second[0] << ',' << q.second[1] << ")]";

                std::cerr << "\nExpected:";
                std::set_difference(result_.begin(), result_.end(), result2.begin(),
                        result2.end(), std::back_inserter(diff));
                printPoints(diff);

                std::cerr << "\nGot:     ";
                std::set_difference(result2.begin(), result2.end(), result_.begin(),
                        result_.end(), std::back_inserter(diff));
                printPoints(diff);
                std::cerr << std::endl;
            }
            result_.clear();
            result2.clear();
        }
        return mismatch;
    }

 private:
    std::vector<Point> dataset_;
    std::vector<std::pair<Point, Point>> queries_;
    std::string params_;
    std::vector<Point> result_;
    bool reporting_query_;

    static void printPoints(std::vector<Point>& points) {
        const size_t size = points.size();
        if (size > 10) points.resize(10);
        for (const auto& p : points)
            std::cerr << " (" << p[0] << ',' << p[1] << ')';
        if (size > points.size())
            std::cerr << " ... and " << size - points.size() << " more";
        points.clear();
    }
};

}  // namespace framework
#endif  // FRAMEWORK_BENCHMARK_H_

