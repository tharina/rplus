// vim: tabstop=2 shiftwidth=2 expandtab

#ifndef RANGE_SEARCH_RPLUS_H_
#define RANGE_SEARCH_RPLUS_H_

#include "range_search.h"



namespace range_search {

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

  };


  class Node {

    private:
      Node* parent_;

    public:
      virtual ~Node() = 0;
      virtual void search(const Rectangle& w, std::vector<Point>& result) = 0;

  };


  class IntermediateNode : public Node {
    
    private:
      struct Entry {
        Rectangle r;
        Node* child;
      };

      unsigned int num_entries_ ;
      Entry entries_[kNodeCapacity];

    public:
      ~IntermediateNode();

      void search(const Rectangle& w, std::vector<Point>& result) override {
        for(int i = 0; i < num_entries_; ++i) {
          if(w.Overlaps(entries_[i].r)) {
            entries_[i].child->search(w, result);
          }
        }
        return;
      }

  };


  class Leaf : public Node {

    private:
      unsigned int num_points_;
      Point points_[kNodeCapacity];

    public:
      ~Leaf();
      
      void search(const Rectangle& w, std::vector<Point>& result) override {
        for(int i = 0; i < num_points_; ++i) {
          if(w.Contains(points_[i])) {
            result.push_back(points_[i]);
          }
        }
      }

  };






  private:
    Node* root;


  public:

    /// Sets the underlying set.
    void assign(const std::vector<Point>& points) override {

    }

    /// Reports all points within the rectangle given by [min, max].
    void reportRange(const Point& min, const Point& max, std::vector<Point>& result) override {
      Rectangle search_window(min, max);

      root->search(search_window, result);
    }

};

}  // namespace range_search
#endif  // RANGE_SEARCH_RPLUSH_
