#include "metrics.hpp"
#include "geo_utils.hpp"
#include <unordered_map>

ClusteringMetrics computeMetrics(const std::vector<Cluster> &clusters,
                                 const std::vector<UserPoint> &users,
                                 double capacite_cible_mbps) {
  ClusteringMetrics metrics;
  metrics.nb_clusters = clusters.size();

  if (clusters.empty() || users.empty()) {
    metrics.avg_fill_rate_percent = 0.0;
    metrics.avg_distance_km = 0.0;
    metrics.memory_footprint_kb = 0.0;
    return metrics;
  }

  // Création d'un dictionnaire pour un accès O(1) aux utilisateurs par leur ID
  std::unordered_map<int, UserPoint> user_map;
  user_map.reserve(users.size());
  for (const auto &u : users) {
    user_map[u.id] = u;
  }

  double total_load = 0.0;
  double total_distance = 0.0;
  int total_users_assigned = 0;

  // Estimation mémoire : taille du vecteur de base + taille de chaque cluster
  size_t mem_bytes =
      sizeof(std::vector<Cluster>) + (clusters.capacity() * sizeof(Cluster));

  for (const auto &c : clusters) {
    total_load += c.current_load;

    // Ajout de la mémoire utilisée par le vecteur interne d'IDs
    mem_bytes += c.users_id.capacity() * sizeof(int);

    for (int uid : c.users_id) {
      auto it = user_map.find(uid);
      if (it != user_map.end()) {
        double dist = haversine(c.center_lat, c.center_lon, it->second.lat,
                                it->second.lon);
        total_distance += dist;
        total_users_assigned++;
      }
    }
  }

  // Calcul du taux de remplissage global : Charge totale / Capacité totale
  // déployée
  double total_capacity_deployed = metrics.nb_clusters * capacite_cible_mbps;
  metrics.avg_fill_rate_percent =
      (total_capacity_deployed > 0)
          ? (total_load / total_capacity_deployed) * 100.0
          : 0.0;

  // Calcul de la distance moyenne
  metrics.avg_distance_km = (total_users_assigned > 0)
                                ? (total_distance / total_users_assigned)
                                : 0.0;

  // Conversion en Kilo-octets
  metrics.memory_footprint_kb = mem_bytes / 1024.0;

  return metrics;
}