// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include <iostream>
#include <limits>

#include "range_search.h"


namespace range_search {

enum class Axis: size_t {
  X = 0,
  Y = 1
};

template<class Point>
class RPlusTree : public RangeSearch<Point> {


  static const size_t kNodeCapacity = 42;
  static const size_t kFillFactor = kNodeCapacity * 1;


  class Rectangle {

    private:
      Point bottom_left_;
      Point top_right_;

    public:

      Rectangle(Point bl, Point tr) : bottom_left_(bl), top_right_(tr) {}

      // TODO rename
      Point& p1() { return bottom_left_; }
      Point& p2() { return top_right_; }

      bool Overlaps(const Rectangle& other) const {
        // TODO float comparison
        return bottom_left_[0] <= other.top_right_[0]   &&
               top_right_[0]   >= other.bottom_left_[0] &&
               top_right_[1]   <= other.bottom_left_[1] &&
               bottom_left_[1] >= other.top_right_[1];

      }

      bool Contains(const Point& p) const {
        // TODO float comparison
        return p[0] >= bottom_left_[0] &&
               p[0] <= top_right_[0]   &&
               p[1] >= bottom_left_[1] &&
               p[1] <= top_right_[1];
      }

      bool Intersects(Axis axis, double distance) {
        // TODO <= ?
        return bottom_left_[axis] < distance && top_right_[axis] > distance;
      }

      double Area() const {
        return (top_right_[0] - bottom_left_[0]) * (top_right_[1] - bottom_left_[1]);
      }

      static Rectangle BoundingBox(const Point* points, size_t length) {
        double xmin = std::numeric_limits<double>::max();
        double xmax = 0;
        double ymin = std::numeric_limits<double>::max();
        double ymax = 0;

        for (int i = 0; i < length; ++i) {
          xmax = std::max(xmax, points[i][Axis::X]);
          xmin = std::max(xmin, points[i][Axis::X]);
          ymax = std::max(ymax, points[i][Axis::Y]);
          ymin = std::max(ymin, points[i][Axis::Y]);
        }

        return Rectangle({xmin, ymin}, {xmax, ymax});
      }

  };

  class IntermediateNode;

  class Node {

    private:
      IntermediateNode* parent_;

    protected:
      // TODO private?
      Rectangle rect_;

    public:
      virtual ~Node() = 0;

      const Rectangle& rectangle() const {
        return rect_;
      }

      virtual void Search(const Rectangle& w, std::vector<Point>& result) const = 0;

      virtual void Split(Axis axis, double offset) = 0;
  };


  class IntermediateNode : public Node {

    private:
      size_t num_entries_ ;
      Node* children_[kNodeCapacity];

    public:

      IntermediateNode(const std::vector<Node*>& children) {
        num_entries_ = children.size();
        for (int i = 0; i < num_entries_; ++i) {
          children_[i] = children[i];
        }
      }

      ~IntermediateNode() override {
        for (int i = 0; i < num_entries_; ++i) {
          delete children_[i];
        }
      }

      void Search(const Rectangle& w, std::vector<Point>& result) const override {
        for (int i = 0; i < num_entries_; ++i) {
          if (w.Overlaps(children_[i]->rectangle())) {
            children_[i]->Search(w, result);
          }
        }
      }

      void Split(Axis axis, double offset) override {
        // TODO
      }

      static Node* Pack(std::vector<Node*> nodes) {
        if (nodes.size() <= kFillFactor) {
          return IntermediateNode(nodes);
        }

        std::vector<Node*> next_level_nodes;
        std::vector<Node*> remainder;
        while (nodes.size() > 0) {
          next_level_nodes.push_back(Partition(nodes, remainder));
          nodes = remainder;
        }

        return Pack(next_level_nodes);
      }

      static IntermediateNode* Partition(std::vector<Node*> set, std::vector<Node*>& remainder) {
        if (set.size() <= kFillFactor) {
          return new IntermediateNode(set);
        }

        double cutline, cutline_x, cutline_y;

        double cost_x = Sweep(set, Axis::X, &cutline_x);
        double cost_y = Sweep(set, Axis::Y, &cutline_y);

        Axis axis;
        if (cost_x < cost_y) {
          axis = Axis::X;
          cutline = cutline_x;
        } else {
          axis = Axis::Y;
          cutline = cutline_y;
        }

        std::vector<Node*> used;

        for (auto node : set) {
          // Check if node has to be split
          if (node->rectangle()->Intersects(axis, cutline)) {
            // Need to split...
            Node* new_node = node->Split(axis, cutline);
            // TODO correct?
            remainder.push_back(new_node);
          }

          // Insert node into correct set
          if (node->rectangle().p1()[axis] < cutline) {
            used.push_back(node);
          } else {
            remainder.push_back(node);
          }
        }

        return new IntermediateNode(used);
      }

      static double Sweep(std::vector<Node*>& set, Axis axis, double& cutline) {
        std::sort(set.begin(), set.end(), [=](Node* a, Node* b) -> bool { return a->rectangle().p1()[axis] < b->rectangle().p1()[axis]; });

        cutline = set[kFillFactor]->rectangle().p1()[axis];

        // Cost: total area around the points
        std::vector<Point> points;
        for (const auto& node : set) {
          points.push_back(node->rectangle().p1());
          points.push_back(node->rectangle().p2());
        }
        return Rectangle::BoundingBox(points.data(), kFillFactor * 2).Area();
      }
  };


  class Leaf : public Node {

    private:
      size_t num_points_;
      Point points_[kNodeCapacity];

    public:
      Leaf(const std::vector<Point>& points) {
        num_points_= points.size();
        for (int i = 0; i < num_points_; ++i) {
          points_[i] = points[i];
        }
      }

      ~Leaf() override { }

      void Search(const Rectangle& w, std::vector<Point>& result) const override {
        for (int i = 0; i < num_points_; ++i) {
          if (w.Contains(points_[i])) {
            result.push_back(points_[i]);
          }
        }
      }

      void Split(Axis axis, double offset) override {
        // TODO
      }

      static Node* Pack(std::vector<Point> points) {
        if (points.size() <= kFillFactor) {
          return Leaf(points);
        }

        std::vector<Node*> leafs;
        std::vector<Point> remainder;
        while (points.size() > 0) {
          leafs.push_back(Partition(points, remainder));
          points = remainder;
        }
        return IntermediateNode::Pack(leafs);
      }

      static Leaf* Partition(const std::vector<Point>& set, std::vector<Point>& remainder) {
        if (set.size() <= kFillFactor) {
          return new Leaf(set);
        }

        double cutline, cutline_x, cutline_y;

        double cost_x = Sweep(set, Axis::X, &cutline_x);
        double cost_y = Sweep(set, Axis::Y, &cutline_y);

        Axis axis;
        if (cost_x < cost_y) {
          axis = Axis::X;
          cutline = cutline_x;
        } else {
          axis = Axis::Y;
          cutline = cutline_y;
        }

        std::vector<Point> used;

        for (const auto& point : set) {
          if (point[axis] < cutline) {
            used.push_back(point);
          } else {
            remainder.push_back(point);
          }
        }

        return new Leaf(used);
      }

      static double Sweep(std::vector<Point>& set, Axis axis, double& cutline) {
        std::sort(set.begin(), set.end(), [=](const Point& a, const Point& b) -> bool { return a[axis] < b[axis]; });

        cutline = set[kFillFactor][axis];

        // Cost: total area around the points
        return Rectangle::BoundingBox(set.data(), kFillFactor).Area();
      }
  };






  private:
    Node* root_;


  public:
    RPlusTree() : root_(nullptr) { }

    ~RPlusTree() override {
      delete root_;
    }

    /// Sets the underlying set.
    // No two points must be equal.
    void assign(const std::vector<Point>& points) override {
      root_ = Leaf::Pack(points);
    }

    /// Reports all points within the rectangle given by [min, max].
    void reportRange(const Point& min, const Point& max, std::vector<Point>& result) override {
      Rectangle search_window(min, max);

      root_->Search(search_window, result);
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
