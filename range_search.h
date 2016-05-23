#ifndef RANGE_SEARCH_H_
#define RANGE_SEARCH_H_

#include <cstddef>
#include <type_traits>
#include <vector>

namespace range_search {

template<class Point>
class RangeSearch {
    static_assert(!std::is_void<
                decltype(std::declval<const Point&>()[std::declval<size_t>()])
            >::value, "Point must support operator[](size_t)");
 public:
    using value_type = Point;

    virtual ~RangeSearch() {}

    /// Sets the underlying set.
    virtual void assign(const std::vector<Point>& points) = 0;

    /// Reports all points within the rectangle given by [min, max].
    virtual void reportRange(const Point& min, const Point& max, std::vector<Point>& result) = 0;

    /// Counts all points within the rectangle given by [min, max].
    virtual size_t countRange(const Point& min, const Point& max) {
        return reportRange(min, max).size();
    }

    /// Alternative interface to reportRange
    std::vector<Point> reportRange(const Point& min, const Point& max) {
        std::vector<Point> vec;
        reportRange(min, max, vec);
        return vec;
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_H_

