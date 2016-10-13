// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include <iostream>
#include <limits>
#include <algorithm>
#include <iomanip>

#include "range_search.h"


namespace range_search {

enum Axis: size_t {
  X = 0,
  Y = 1
};

template<class Point>
class RPlusTree : public RangeSearch<Point> {


  static const size_t kNodeCapacity = 4;
  static const size_t kFillFactor = kNodeCapacity * 1;


  class Rectangle {

    private:
      Point bottom_left_;
      Point top_right_;

    public:

      Rectangle(Point bl, Point tr) : bottom_left_(bl), top_right_(tr) {}

      Rectangle() { }

      // TODO rename
      const Point& p1() const { return bottom_left_; }
      const Point& p2() const { return top_right_; }

      bool Overlaps(const Rectangle& other) const {
        // TODO float comparison
        return bottom_left_[0] <= other.top_right_[0]   &&
               top_right_[0]   >= other.bottom_left_[0] &&
               top_right_[1]   >= other.bottom_left_[1] &&
               bottom_left_[1] <= other.top_right_[1];
      }

      bool Contains(const Point& p) const {
        // TODO float comparison
        return p[0] >= bottom_left_[0] &&
               p[0] <= top_right_[0]   &&
               p[1] >= bottom_left_[1] &&
               p[1] <= top_right_[1];
      }

      bool Intersects(Axis axis, double distance) const {
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
          xmin = std::min(xmin, points[i][Axis::X]);
          ymax = std::max(ymax, points[i][Axis::Y]);
          ymin = std::min(ymin, points[i][Axis::Y]);
        }

        return Rectangle({xmin, ymin}, {xmax, ymax});
      }

      static Rectangle BoundingBox(const std::vector<Point>& points) {
        return BoundingBox(points.data(), points.size());
      }

      void Print() const {
        std::cout << std::fixed << std::setprecision(2) <<
                     "(" << bottom_left_[0] << "," << bottom_left_[1] << ") " <<
                     "(" << top_right_[0]   << "," << top_right_[1]   << ")";
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
      Node() { }

      virtual ~Node() { }

      const Rectangle& rectangle() const {
        return rect_;
      }

      virtual void Search(const Rectangle& w, std::vector<Point>& result) const = 0;

      virtual Node* Split(Axis axis, double distance) = 0;

      virtual void Print(size_t indent_level) = 0;
  };


  class IntermediateNode : public Node {

    private:
      size_t num_children_ ;
      Node* children_[kNodeCapacity];

    public:

      IntermediateNode(const std::vector<Node*>& children) : Node() {
        num_children_ = children.size();
        for (int i = 0; i < num_children_; ++i) {
          children_[i] = children[i];
        }

        // Calculate bounding box
        std::vector<Point> points;
        for (const auto& node : children) {
          points.push_back(node->rectangle().p1());
          points.push_back(node->rectangle().p2());
        }
        Node::rect_ = Rectangle::BoundingBox(points);
      }

      ~IntermediateNode() override {
        for (int i = 0; i < num_children_; ++i) {
          delete children_[i];
        }
      }

      void Search(const Rectangle& w, std::vector<Point>& result) const override {
        for (int i = 0; i < num_children_; ++i) {
          if (w.Overlaps(children_[i]->rectangle())) {
            children_[i]->Search(w, result);
          }
        }
      }

      Node* Split(Axis axis, double distance) override {
        std::vector<Node*> keep, abandon;

        for (int i = 0; i < num_children_; ++i) {
          Node* node = children_[i];
          if (node->rectangle().p2()[axis] <= distance) {
            keep.push_back(node);
          } else if (node->rectangle().p1()[axis] < distance) {
            Node* new_node = node->Split(axis, distance);
            // TODO correct?
            keep.push_back(node);
            abandon.push_back(new_node);
          } else {
            abandon.push_back(node);
          }
        }

        // TODO make better
        std::vector<Point> points;
        num_children_ = keep.size();
        for (int i = 0; i < num_children_; ++i) {
          children_[i] = keep[i];
          points.push_back(keep[i]->rectangle().p1());
          points.push_back(keep[i]->rectangle().p2());
        }

        // Update bounding box
        Node::rect_ = Rectangle::BoundingBox(points);

        return new IntermediateNode(abandon);
      }

      static Node* Pack(std::vector<Node*> nodes) {
        if (nodes.size() <= kFillFactor) {
          return new IntermediateNode(nodes);
        }

        std::vector<Node*> next_level_nodes;
        std::vector<Node*> remainder;
        while (nodes.size() > 0) {
          next_level_nodes.push_back(Partition(nodes, remainder));
          nodes.clear();
          nodes.swap(remainder);
        }

        return Pack(next_level_nodes);
      }

      static IntermediateNode* Partition(std::vector<Node*> set, std::vector<Node*>& remainder) {
        if (set.size() <= kFillFactor) {
          return new IntermediateNode(set);
        }

        double cutline, cutline_x, cutline_y;

        double cost_x = Sweep(set, Axis::X, cutline_x);
        double cost_y = Sweep(set, Axis::Y, cutline_y);

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
          if (node->rectangle().Intersects(axis, cutline)) {
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

      void Print(size_t indent_level) {
        for (size_t i = 1; i < indent_level; ++i) {
          std::cout << "|  ";
        }
        if (indent_level > 0) {
          std::cout << "---";
        }
        std::cout << "# [" << num_children_ << "]  ";
        Node::rectangle().Print();
        std::cout << std::endl;

        for (size_t i = 0; i < num_children_; ++i) {
          children_[i]->Print(indent_level + 1);
        }

      }
  };


  class Leaf : public Node {

    private:
      size_t num_points_;
      Point points_[kNodeCapacity];

    public:
      Leaf(const std::vector<Point>& points) : Node() {
        num_points_= points.size();
        for (int i = 0; i < num_points_; ++i) {
          points_[i] = points[i];
        }

        // Calculate bounding box
        Node::rect_ = Rectangle::BoundingBox(points);
      }

      ~Leaf() override { }

      void Search(const Rectangle& w, std::vector<Point>& result) const override {
        for (int i = 0; i < num_points_; ++i) {
          if (w.Contains(points_[i])) {
            result.push_back(points_[i]);
          }
        }
      }

      Node* Split(Axis axis, double distance) override {
        std::vector<Point> keep, abandon;

        for (int i = 0; i < num_points_; ++i) {
          const Point& point = points_[i];
          if (point[axis] < distance) {
            keep.push_back(point);
          } else {
            abandon.push_back(point);
          }
        }

        // TODO make better
        num_points_ = keep.size();
        for (int i = 0; i < num_points_; ++i) {
          points_[i] = keep[i];
        }

        // Update bounding box
        Node::rect_ = Rectangle::BoundingBox(keep);

        return new Leaf(abandon);
      }

      static Node* Pack(std::vector<Point> points) {
        if (points.size() <= kFillFactor) {
          return new Leaf(points);
        }

        std::vector<Node*> leafs;
        std::vector<Point> remainder;
        while (points.size() > 0) {
          leafs.push_back(Partition(points, remainder));
          points.clear();
          points.swap(remainder);
        }
        return IntermediateNode::Pack(leafs);
      }

      static Leaf* Partition(std::vector<Point>& set, std::vector<Point>& remainder) {
        if (set.size() <= kFillFactor) {
          return new Leaf(set);
        }

        double cutline, cutline_x, cutline_y;

        double cost_x = Sweep(set, Axis::X, cutline_x);
        double cost_y = Sweep(set, Axis::Y, cutline_y);

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


      void Print(size_t indent_level) {
        for (size_t i = 1; i < indent_level; ++i) {
          std::cout << "|  ";
        }
        if (indent_level > 0) {
          std::cout << "|--";
        }
        std::cout << " [" << num_points_ << "]  ";
        Node::rectangle().Print();
        std::cout << std::endl;

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

    void Print() {
      root_->Print(0);
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
