#include "quadtree.hpp"
#include <algorithm>

void Quadtree::subdivide() {
  double nextW = boundary.w / 2.0;
  double nextH = boundary.h / 2.0;

  // Création des 4 zones (Nord-Ouest, Nord-Est, Sud-Ouest, Sud-Est)
  nw = std::make_unique<Quadtree>(
      Boundary{boundary.x - nextW, boundary.y + nextH, nextW, nextH});
  ne = std::make_unique<Quadtree>(
      Boundary{boundary.x + nextW, boundary.y + nextH, nextW, nextH});
  sw = std::make_unique<Quadtree>(
      Boundary{boundary.x - nextW, boundary.y - nextH, nextW, nextH});
  se = std::make_unique<Quadtree>(
      Boundary{boundary.x + nextW, boundary.y - nextH, nextW, nextH});

  divided = true;

  // On déplace les points actuels vers les enfants
  for (const auto &p : points) {
    if (nw->insert(p))
      continue;
    if (ne->insert(p))
      continue;
    if (sw->insert(p))
      continue;
    if (se->insert(p))
      continue;
  }
  points.clear();
}

bool Quadtree::insert(const UserPoint &p) {
  if (!boundary.contains(p)) {
    return false;
  }

  if (!divided) {
    if (points.size() < CAPACITY) {
      points.push_back(p);
      return true;
    }
    subdivide();
  }

  // Tentative d'insertion dans les enfants
  return (nw->insert(p) || ne->insert(p) || sw->insert(p) || se->insert(p));
}

void Quadtree::queryRange(const Boundary &range,
                          std::vector<UserPoint> &found) const {
  if (!boundary.intersects(range)) {
    return;
  }

  if (divided) {
    nw->queryRange(range, found);
    ne->queryRange(range, found);
    sw->queryRange(range, found);
    se->queryRange(range, found);
  } else {
    for (const auto &p : points) {
      if (range.contains(p)) {
        found.push_back(p);
      }
    }
  }
}