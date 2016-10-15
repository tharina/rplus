// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iomanip>

#define DEBUG 0

#include "range_search.h"
#include "geometry.h"

namespace range_search {


template<class Point>
class RPlusTree : public RangeSearch<Point> {

  static const size_t kNodeCapacity = 32;
  static const size_t kFillFactor = kNodeCapacity * 1;

  // Forward declaration required for struct Entry.
  class Node;

  // An entry in the tree consists of a child node pointer and its bounding box.
  struct Entry {
    Entry() { }
    Entry(Node* n, const Rectangle<Point>& r) : node(n), rectangle(r) { }

    Node* node;
    Rectangle<Point> rectangle;
  };

  //
  // Cost metrics used by SWEEP.
  //

  // Region with the smallest area is preferred.
  static double TotalAreaCost(const std::vector<Entry>& entries_sorted, size_t num_used, Axis axis, double cutline) {
    // Unused arguments
    (void)axis;
    (void)cutline;

    // Cost: total area around the points
    std::vector<Point> points;
    for (size_t i = 0; i < num_used; ++i) {
      points.push_back(entries_sorted[i].rectangle.bottom_left());
      points.push_back(entries_sorted[i].rectangle.top_right());
    }
    return Rectangle<Point>::BoundingBox(points).Area();
  }

  // Region that requires the least number of rectangle splits is preferred.
  static double NumRectangleCuts(const std::vector<Entry>& entries_sorted, size_t num_used, Axis axis, double cutline) {
    double num_splits = 0;
    for (size_t i = 0; i < num_used; ++i) {
      num_splits += entries_sorted[i].rectangle.Intersects(axis, cutline);
    }
    return num_splits;
  }

  // See comment in Sweep(). No function pointers for performance reasons.
  #define COST TotalAreaCost

  // Nodes of the tree.
  class Node {
    public:
      Node(const std::vector<Entry>& entries) : point_count_(0) {
        Assert(entries.size() > 0);         // Nodes must always have at least one entry.
        num_entries_ = entries.size();
        std::copy(std::begin(entries), std::end(entries), std::begin(entries_));
        CalcCount();
      }

      ~Node() {
        for (size_t i = 0; i < num_entries_; ++i) {
          delete entries_[i].node;
        }
      }


      // Return the number of points stored in the subtree of node
      size_t point_count() const { return point_count_; }

      // Calculate the number of points in the subtree of the node
      void CalcCount() {
        for(size_t i = 0; i < num_entries_; ++i) {
          Node* child = entries_[i].node;
          if(child) {
            point_count_ += child->point_count();
          } else {
            point_count_ += 1;
          }
        }
      }

      // Return whether this node is a leaf.
      bool is_leaf() const {
        return entries_[0].node == nullptr;
      }

      // Compute the bounding box for all entries of this node.
      Rectangle<Point> ComputeBoundingBox() const {
        std::vector<Point> points;
        for (size_t i = 0; i < num_entries_; ++i) {
          points.push_back(entries_[i].rectangle.bottom_left());
          points.push_back(entries_[i].rectangle.top_right());
        }
        return Rectangle<Point>::BoundingBox(points);
      }

      // Recursively (if !is_leaf) search for all entries covered by this node that
      // overlap with the given search window.
      void Search(const Rectangle<Point>& search_window, std::vector<Point>& result) const {
        bool leaf = is_leaf();
        for (size_t i = 0; i < num_entries_; ++i) {
          if (search_window.Overlaps(entries_[i].rectangle)) {
            if (leaf) {
              result.push_back(entries_[i].rectangle.bottom_left());
            } else {
              entries_[i].node->Search(search_window, result);
            }
          }
        }
      }
      
      // Recursively (if !is_leaf) count all entries covered by this node that
      // overlap with the given search window.
      size_t Count(const Rectangle<Point>& search_window) const {
        size_t count = 0;
        bool leaf = is_leaf();
        for (size_t i = 0; i < num_entries_; ++i) {
          const Entry& entry = entries_[i];
          if (search_window.Contains(entry.rectangle)) {
            if (leaf) {
              count += 1;
            } else {
              //count += entry.node->Count(search_window);
              count += entry.node->point_count();
            }
          } else if (search_window.Overlaps(entry.rectangle)) {
            Assert(!leaf);
            count += entry.node->Count(search_window);
          }
        }
        return count;
      }


      // Split this node along an axis-aligned line. Return a new node containing the entries that are no longer part of this node after the split.
      // The node's bounding box has to be recalculated after splitting.
      Node* Split(Axis axis, double offset) {
        Assert(num_entries_ > 0);
        std::vector<Entry> abandon;

        size_t new_num_entries = 0;
        for (size_t i = 0; i < num_entries_; ++i) {
          Entry& entry = entries_[i];
          if (entry.rectangle.max_side(axis) <= offset) {
            entries_[new_num_entries++] = entry;
          } else if (entry.rectangle.min_side(axis) < offset) {
            Assert(!is_leaf());               // Currently cannot happen since we only store points in the leaves.

            // Need to split the child node.
            Node* new_node = entry.node->Split(axis, offset);

            // The original node will now contain all entries "smaller" than the split line. Its bounding box has to be recomputed though.
            entries_[new_num_entries].node = entry.node;
            entries_[new_num_entries++].rectangle = entry.node->ComputeBoundingBox();

            // The new node will contain all other entries, so "abandon" it.
            abandon.emplace_back(new_node, new_node->ComputeBoundingBox());
          } else {
            abandon.push_back(entry);
          }
        }

        num_entries_ = new_num_entries;

        Node* next = new Node(abandon);

        // Remove points moved to the new node from this point's counter
        point_count_ -= next->point_count();
        
        return next;
      }

      void Print(size_t indent_level) {
        for (size_t i = 1; i < indent_level; ++i) {
          std::cout << "|  ";
        }
        if (indent_level > 0) {
          std::cout << "---";
        }
        std::cout << "# [" << num_entries_ << "]  " << ComputeBoundingBox() << "    #"<<point_count_<<"#"<<std::endl;

        for (size_t i = 0; i < num_entries_; ++i) {
          Node* child = entries_[i].node;
          if(child) {
            entries_[i].node->Print(indent_level + 1);
          }
        }
      }

    private:
      // Node entries. Entries are stored inline for improved performance.
      size_t num_entries_;
      // Number of points stored in the subtree of the node
      size_t point_count_;

      Entry entries_[kNodeCapacity];
  };

  private:
    Node* root_;

    // Pack a set of entries into a new R+ (sub-)tree.
    static Node* Pack(std::vector<Entry>& entries) {
      if (entries.size() <= kFillFactor) {
        return new Node(entries);
      }

      std::vector<Entry> next_level_entries;
      std::vector<Entry> remainder;
      while (entries.size() > 0) {
        next_level_entries.push_back(Partition(entries, remainder));
        entries.clear();
        entries.swap(remainder);
      }

      return Pack(next_level_entries);
    }

    static Entry Partition(std::vector<Entry>& set, std::vector<Entry>& remainder) {
      if (set.size() <= kFillFactor) {
        Node* node = new Node(set);
      return Entry(node, node->ComputeBoundingBox());
      }

      double cutline, cutline_x, cutline_y;
      double cost_x = Sweep(set, Axis::X, cutline_x);
      double cost_y = Sweep(set, Axis::Y, cutline_y);

      // Determine cheapest cutline.
      Axis axis;
      if (cost_x < cost_y) {
        axis = Axis::X;
        cutline = cutline_x;
      } else {
        axis = Axis::Y;
        cutline = cutline_y;
      }

      std::vector<Entry> used;
      for (auto& entry : set) {
        if (entry.rectangle.Intersects(axis, cutline)) {
          // Need to split the node and add the newly created node to the remainder set.
          Assert(entry.node->ComputeBoundingBox() == entry.rectangle);
          Node* new_node = entry.node->Split(axis, cutline);
          entry.rectangle = entry.node->ComputeBoundingBox();
          remainder.emplace_back(new_node, new_node->ComputeBoundingBox());
        }

        // Insert node into correct set
        if (entry.rectangle.min_side(axis) < cutline) {
          used.push_back(entry);
        } else {
          remainder.push_back(entry);
        }
      }

      Node* node = new Node(used);
      return Entry(node, node->ComputeBoundingBox());
    }

    static double Sweep(std::vector<Entry>& set, Axis axis, double& cutline) {
      Assert(set.size() > 0);

      std::sort(set.begin(), set.end(), [=](const Entry& a, const Entry& b) -> bool { return a.rectangle.min_side(axis) < b.rectangle.min_side(axis); });

      cutline = set[kFillFactor].rectangle.min_side(axis);

      // Check for edge case: all points on an axis-aligned line.
      if (set[0].rectangle.min_side(axis) == set[set.size() - 1].rectangle.min_side(axis)) {
        return std::numeric_limits<double>::max();
      }

      // This should be a template parameter, but that causes issues due to internal linkage of the template arguments...
      return COST(set, kFillFactor, axis, cutline);
    }

  public:
    RPlusTree() : root_(nullptr) { }

    ~RPlusTree() override {
      delete root_;
    }

    /// Sets the underlying set.
    // No two points must be equal.
    void assign(const std::vector<Point>& points) override {
      std::vector<Entry> entries(points.size());
      for (size_t i = 0; i < points.size(); ++i) {
        const Point& p = points[i];
        entries[i] = Entry(nullptr, Rectangle<Point>(p, p));
      }
      root_ = Pack(entries);
    }

    /// Reports all points within the rectangle given by [min, max].
    void reportRange(const Point& min, const Point& max, std::vector<Point>& result) override {
      Rectangle<Point> search_window(min, max);
      root_->Search(search_window, result);
    } 
    
    /// Counts all points within the rectangle given by [min, max].
    size_t countRange(const Point& min, const Point& max) override {
      Rectangle<Point> search_window(min, max);
      return root_->Count(search_window);
    }

    void Print() {
      root_->Print(0);
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
