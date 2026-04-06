#include "hilbert.hpp"
#include <algorithm>
#include <cmath>

// Fonction utilitaire de rotation pour l'algorithme de Hilbert
static void rot(int n, int *x, int *y, int rx, int ry) {
  if (ry == 0) {
    if (rx == 1) {
      *x = n - 1 - *x;
      *y = n - 1 - *y;
    }
    int t = *x;
    *x = *y;
    *y = t;
  }
}

// Convertit des coordonnées (x,y) sur une grille NxN en une distance de Hilbert
static int xy2d(int n, int x, int y) {
  int rx, ry, s, d = 0;
  for (s = n / 2; s > 0; s /= 2) {
    rx = (x & s) > 0;
    ry = (y & s) > 0;
    d += s * s * ((3 * rx) ^ ry);
    rot(s, &x, &y, rx, ry);
  }
  return d;
}

void sortIndicesByHilbert(const std::vector<UserPoint> &users,
                          std::vector<int> &indices) {
  if (users.empty())
    return;

  // 1. Trouver la "Bounding Box" (Boîte englobante) de tous les utilisateurs
  double min_lon = users[0].lon, max_lon = users[0].lon;
  double min_lat = users[0].lat, max_lat = users[0].lat;

  for (const auto &u : users) {
    if (u.lon < min_lon)
      min_lon = u.lon;
    if (u.lon > max_lon)
      max_lon = u.lon;
    if (u.lat < min_lat)
      min_lat = u.lat;
    if (u.lat > max_lat)
      max_lat = u.lat;
  }

  // 2. Définir la résolution de la grille (2^16 = 65536 cases)
  int order = 16;
  int n = 1 << order;

  // 3. Calculer la distance de Hilbert pour chaque utilisateur
  std::vector<long long> h_dist(users.size());
  for (size_t i = 0; i < users.size(); ++i) {
    // Normalisation des GPS (float) vers la grille (int) [0, n-1]
    int x = (int)((users[i].lon - min_lon) / (max_lon - min_lon) * (n - 1));
    int y = (int)((users[i].lat - min_lat) / (max_lat - min_lat) * (n - 1));

    h_dist[i] = xy2d(n, x, y);
  }

  // 4. Trier les indices fournis en fonction de cette distance
  std::sort(indices.begin(), indices.end(),
            [&](int a, int b) { return h_dist[a] < h_dist[b]; });
}