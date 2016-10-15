// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "range_search.h"
#include "geometry.h"

#define DEBUG 1


namespace range_search {


template<class Point>
class RPlusTree : public RangeSearch<Point> {


  static const size_t kNodeCapacity = 8;
  static const size_t kFillFactor = kNodeCapacity * 1;



  class IntermediateNode;

  class Node {

    private:
      IntermediateNode* parent_;

    protected:
      // TODO private?
      Rectangle<Point> rect_;

    public:
      Node() { }

      virtual ~Node() { }

      const Rectangle<Point>& rectangle() const {
        return rect_;
      }

      virtual void Search(const Rectangle<Point>& w, std::vector<Point>& result) const = 0;

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
        for (size_t i = 0; i < num_children_; ++i) {
          children_[i] = children[i];
        }

        // Calculate bounding box
        std::vector<Point> points;
        for (const auto& node : children) {
          points.push_back(node->rectangle().bottom_left());
          points.push_back(node->rectangle().top_right());
        }
        Node::rect_ = Rectangle<Point>::BoundingBox(points);
      }

      ~IntermediateNode() override {
        for (size_t i = 0; i < num_children_; ++i) {
          delete children_[i];
        }
      }

      void Search(const Rectangle<Point>& w, std::vector<Point>& result) const override {
        for (size_t i = 0; i < num_children_; ++i) {
          if (w.Overlaps(children_[i]->rectangle())) {
            children_[i]->Search(w, result);
          }
        }
      }

      Node* Split(Axis axis, double distance) override {
        std::vector<Node*> keep, abandon;

        for (size_t i = 0; i < num_children_; ++i) {
          Node* node = children_[i];
          if (node->rectangle().top_right()[axis] <= distance) {
            keep.push_back(node);
          } else if (node->rectangle().bottom_left()[axis] < distance) {
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
        for (size_t i = 0; i < num_children_; ++i) {
          children_[i] = keep[i];
          points.push_back(keep[i]->rectangle().bottom_left());
          points.push_back(keep[i]->rectangle().top_right());
        }

        // Update bounding box
        Node::rect_ = Rectangle<Point>::BoundingBox(points);

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
          if (node->rectangle().bottom_left()[axis] < cutline) {
            used.push_back(node);
          } else {
            remainder.push_back(node);
          }
        }

        return new IntermediateNode(used);
      }

      static double Sweep(std::vector<Node*>& set, Axis axis, double& cutline) {
        std::sort(set.begin(), set.end(), [=](Node* a, Node* b) -> bool { return a->rectangle().bottom_left()[axis] < b->rectangle().bottom_left()[axis]; });

        cutline = set[kFillFactor]->rectangle().bottom_left()[axis];

        // Check for edge case: all points on a axis-aligned line.
        if (set.begin()->rectangle().p1()[axis] == set.end()->rectangle().p1()[axis]) {
          return std::numeric_limits<double>::max();
        }

        // Cost: total area around the points
        std::vector<Point> points;
        for (const auto& node : set) {
          points.push_back(node->rectangle().bottom_left());
          points.push_back(node->rectangle().top_right());
        }
        return Rectangle<Point>::BoundingBox(points.data(), kFillFactor * 2).Area();
      }

      void Print(size_t indent_level) override {
        for (size_t i = 1; i < indent_level; ++i) {
          std::cout << "|  ";
        }
        if (indent_level > 0) {
          std::cout << "---";
        }
        std::cout << "# [" << num_children_ << "]  " << Node::rect_ << std::endl;

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
        num_points_ = points.size();
        for (size_t i = 0; i < num_points_; ++i) {
          points_[i] = points[i];
        }

        // Calculate bounding box
        Node::rect_ = Rectangle<Point>::BoundingBox(points);
      }

      ~Leaf() override { }

      void Search(const Rectangle<Point>& w, std::vector<Point>& result) const override {
        for (size_t i = 0; i < num_points_; ++i) {
          if (w.Contains(points_[i])) {
            result.push_back(points_[i]);
          }
        }
      }

      Node* Split(Axis axis, double distance) override {
        std::vector<Point> keep, abandon;

        for (size_t i = 0; i < num_points_; ++i) {
          const Point& point = points_[i];
          if (point[axis] < distance) {
            keep.push_back(point);
          } else {
            abandon.push_back(point);
          }
        }

        // TODO make better
        num_points_ = keep.size();
        for (size_t i = 0; i < num_points_; ++i) {
          points_[i] = keep[i];
        }

        // Update bounding box
        Node::rect_ = Rectangle<Point>::BoundingBox(keep);

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
        return Rectangle<Point>::BoundingBox(set.data(), kFillFactor).Area();
      }


      void Print(size_t indent_level) override {
        for (size_t i = 1; i < indent_level; ++i) {
          std::cout << "|  ";
        }
        if (indent_level > 0) {
          std::cout << "|--";
        }
        std::cout << " [" << num_points_ << "]  " << Node::rect_ << std::endl;
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
      Rectangle<Point> search_window(min, max);
      root_->Search(search_window, result);
    }

    void Print() {
      root_->Print(0);
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
