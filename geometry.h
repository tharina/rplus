
// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <iostream>
#include <limits>
#include <iomanip>

#if DEBUG
  #define Assert(expr) if (!(expr)) { std::cerr << "Assertion \"" << #expr << "\" failed (in " << __FILE__ << ":" << __LINE__ << "). Aborting" << std::endl; abort(); }
#else
  #define Assert(expr) if (!(expr)) { }
#endif

enum Axis: size_t {
  X = 0,
  Y = 1
};


template<class Point>
class Rectangle {

private:
  Point bottom_left_;
  Point top_right_;

public:

  Rectangle(Point bl, Point tr) : bottom_left_(bl), top_right_(tr) {}

  Rectangle() { }

  const Point& bottom_left() const { return bottom_left_; }
  const Point& top_right() const { return top_right_; }

  double min_side(Axis axis) const { return bottom_left_[axis]; }
  double max_side(Axis axis) const { return top_right_[axis]; }

  bool Overlaps(const Rectangle& other) const {
    return bottom_left_[0] <= other.top_right_[0]   &&
           top_right_[0]   >= other.bottom_left_[0] &&
           top_right_[1]   >= other.bottom_left_[1] &&
           bottom_left_[1] <= other.top_right_[1];
  }

  bool Contains(const Point& p) const {
    return p[0] >= bottom_left_[0] &&
           p[0] <= top_right_[0]   &&
           p[1] >= bottom_left_[1] &&
           p[1] <= top_right_[1];
  }

  bool Intersects(Axis axis, double offset) const {
    return bottom_left_[axis] < offset && top_right_[axis] > offset;
  }

  double Area() const {
    return (top_right_[0] - bottom_left_[0]) * (top_right_[1] - bottom_left_[1]);
  }

  static Rectangle BoundingBox(const Point* points, size_t length) {
    Assert(length > 0);
    double xmin = std::numeric_limits<double>::max();
    double xmax = std::numeric_limits<double>::lowest();
    double ymin = std::numeric_limits<double>::max();
    double ymax = std::numeric_limits<double>::lowest();

    for (size_t i = 0; i < length; ++i) {
      xmax = std::max(xmax, points[i][Axis::X]);
      xmin = std::min(xmin, points[i][Axis::X]);
      ymax = std::max(ymax, points[i][Axis::Y]);
      ymin = std::min(ymin, points[i][Axis::Y]);
    }

    return Rectangle({{xmin, ymin}}, {{xmax, ymax}});
  }

  static Rectangle BoundingBox(const std::vector<Point>& points) {
    return BoundingBox(points.data(), points.size());
  }

  bool operator==(const Rectangle& other) const {
    return bottom_left_[0] == other.bottom_left_[0] &&
           bottom_left_[1] == other.bottom_left_[1] &&
           top_right_[0] == other.top_right_[0] &&
           top_right_[1] == other.top_right_[1];
  }

  friend std::ostream& operator<<(std::ostream& os, const Rectangle& r) {
    os << std::fixed << std::setprecision(2)
       << "(" << r.bottom_left_[0] << "," << r.bottom_left_[1] << ") "
       << "(" << r.top_right_[0]   << "," << r.top_right_[1]   << ")";
    return os;
  }
};

#endif  // GEOMETRY_H_
