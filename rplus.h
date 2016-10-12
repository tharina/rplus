// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include "range_search.h"



namespace range_search {

const size_t kFillFactor = 42;


struct Point2D {
  double x;
  double y;
};


class Rectangle {

  private:
    Point2D bottom_left_;
    Point2D top_right;

  public:
    bool Intersects(Rectangle other);
    bool Contains(Point2D p);

};


class Node {

  private:
    Node* parent_;

  public:
    virtual ~Node() = 0;

};


class IntermediateNode : public Node {
  
  private:
    struct Entry {
      Rectangle r;
      Node* child;
    };

    unsigned int num_entries_;
    Entry entries_[kFillFactor];

  public:
    virtual ~IntermediateNode();

};


class Leaf : public Node {

  private:
    unsigned int num_points_;
    Point2D points_[kFillFactor];

  public:
    virtual ~Leaf();
};


template<class Point>
class RPlusTree : public RangeSearch<Point> {

  private:
    Node* root;
        


  public:
    using value_type = Point2D;


    /// Sets the underlying set.
    void assign(const std::vector<Point>& points) override {

    }

    /// Reports all points within the rectangle given by [min, max].
    void reportRange(const Point& min, const Point& max, std::vector<Point>& result) override {
        
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
