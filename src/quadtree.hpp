/**
 * @file quadtree.hpp
 * @author Arthur
 * @brief
 * @version 0.1
 * @date 2026-03-21
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "csv_read.hpp"
#include <memory>
#include <vector>

/**
 * @brief Boîte englobante pour une zone géographique
 */
struct Boundary {
  double x, y, w, h; // x,y = centre ; w,h = demi-dimensions
  bool contains(const UserPoint &p) const {
    return (p.lon >= x - w && p.lon <= x + w && p.lat >= y - h &&
            p.lat <= y + h);
  }

  bool intersects(const Boundary &other) const {
    return !(other.x - other.w > x + w || other.x + other.w < x - w ||
             other.y - other.h > y + h || other.y + other.h < y - h);
  }
};

class Quadtree {
private:
  static const int CAPACITY = 50; // Points max avant division
  Boundary boundary;
  std::vector<UserPoint> points;

  // Enfants
  std::unique_ptr<Quadtree> nw, ne, sw, se;
  bool divided = false;

  void subdivide();

public:
  Quadtree(Boundary b) : boundary(b) {}

  bool insert(const UserPoint &p);

  void queryRange(const Boundary &range, std::vector<UserPoint> &found) const;
};