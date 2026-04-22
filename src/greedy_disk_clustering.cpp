#include "greedy_disk_clustering.hpp"
#include "geo_utils.hpp"
#include "shift.hpp"
#include <numeric>
#include <unordered_map>

static bool mergeRespectRadius(const Cluster &ci, const Cluster &cj,
                                const std::vector<UserPoint> &users,
                                double new_lat, double new_lon,
                                const std::unordered_map<int, int> &id_to_idx) {
    for (int uid : ci.users_id) {
        auto it = id_to_idx.find(uid);
        if (it == id_to_idx.end()) continue;
        if (haversine(new_lat, new_lon,
                      users[it->second].lat,
                      users[it->second].lon) > RAYON_KM)
            return false;
    }
    for (int uid : cj.users_id) {
        auto it = id_to_idx.find(uid);
        if (it == id_to_idx.end()) continue;
        if (haversine(new_lat, new_lon,
                      users[it->second].lat,
                      users[it->second].lon) > RAYON_KM)
            return false;
    }
    return true;
}

std::vector<Cluster> mergeClusters(std::vector<Cluster> clusters,
                                   const std::vector<UserPoint> &users,
                                   double capacite_cible_mbps,
                                   Strategie_t strategie) {

    // Construire la map id -> index une seule fois
    std::unordered_map<int, int> id_to_idx;
    id_to_idx.reserve(users.size());
    for (int i = 0; i < (int)users.size(); ++i)
        id_to_idx[users[i].id] = i;

    bool merged = true;
    while (merged) {
        merged = false;

        Boundary world = {0.0, 0.0, 180.0, 90.0};
        Quadtree tree(world);
        for (size_t i = 0; i < clusters.size(); ++i) {
            UserPoint p;
            p.id  = (int)i;
            p.lat = clusters[i].center_lat;
            p.lon = clusters[i].center_lon;
            tree.insert(p);
        }

        std::vector<bool> to_remove(clusters.size(), false);

        for (size_t i = 0; i < clusters.size(); ++i) {
            if (to_remove[i]) continue;

            double lat_rad   = clusters[i].center_lat * M_PI / 180.0;
            double range_lat = (RAYON_KM * 2.0) / 111.0;
            double range_lon = (RAYON_KM * 2.0) / (111.0 * std::cos(lat_rad));

            Boundary range = {clusters[i].center_lon, clusters[i].center_lat,
                              range_lon, range_lat};

            std::vector<UserPoint> candidates;
            tree.queryRange(range, candidates);

            double load_i = (strategie == PESSIMISTE) ? clusters[i].sum_pir
                          : (strategie == OPTIMISTE)  ? clusters[i].sum_cir
                          : 0.7 * clusters[i].sum_cir + 0.3 * clusters[i].sum_pir;

            for (const auto &cand : candidates) {
                size_t j = (size_t)cand.id;
                if (j == i || to_remove[j]) continue;

                double dist = haversine(clusters[i].center_lat, clusters[i].center_lon,
                                        clusters[j].center_lat, clusters[j].center_lon);
                if (dist > RAYON_KM * 2.0) continue;

                double load_j = (strategie == PESSIMISTE) ? clusters[j].sum_pir
                              : (strategie == OPTIMISTE)  ? clusters[j].sum_cir
                              : 0.7 * clusters[j].sum_cir + 0.3 * clusters[j].sum_pir;

                if (load_i + load_j > capacite_cible_mbps) continue;

                size_t ni = clusters[i].users_id.size();
                size_t nj = clusters[j].users_id.size();
                double new_lat = (clusters[i].center_lat * ni +
                                  clusters[j].center_lat * nj) / (ni + nj);
                double new_lon = (clusters[i].center_lon * ni +
                                  clusters[j].center_lon * nj) / (ni + nj);

                if (!mergeRespectRadius(clusters[i], clusters[j],
                                        users, new_lat, new_lon, id_to_idx))
                    continue;

                clusters[i].center_lat  = new_lat;
                clusters[i].center_lon  = new_lon;
                clusters[i].users_id.insert(clusters[i].users_id.end(),
                                            clusters[j].users_id.begin(),
                                            clusters[j].users_id.end());
                clusters[i].sum_pir     += clusters[j].sum_pir;
                clusters[i].sum_cir     += clusters[j].sum_cir;
                clusters[i].current_load = load_i + load_j;
                load_i                  += load_j;

                to_remove[j] = true;
                merged = true;
            }
        }

        std::vector<Cluster> next;
        next.reserve(clusters.size());
        for (size_t i = 0; i < clusters.size(); ++i)
            if (!to_remove[i])
                next.push_back(std::move(clusters[i]));
        clusters = std::move(next);
    }
    return clusters;
}

std::vector<Cluster> runGreedyClustering(const std::vector<UserPoint> &users,
                                         Strategie_t strategie,
                                         int remplissage,
                                         bool use_merge) {
  // Initialisation de la liste de cluster
  std::vector<Cluster> clusters;
  int n = users.size();

  if (n == 0) {
    return clusters;
  }

  // Masque pour savoir qui est déjà dans un cluster
  std::vector<bool> assigned(n, false);
  int assigned_count = 0;
  int cluster_id_gen = 1;

  // Calcul de la capacité effective en Mbps selon le paramètre remplissage
  double capacite_cible_mbps = (CAP_MAX_GBPS * 1000.0) * (remplissage / 100.0);

  // Boucle principale
  while (assigned_count < n) {
    // Recherche du pivot (le premier non assigné)
    int pivot_idx = -1;
    for (int i = 0; i < n; ++i) {
      if (!assigned[i]) {
        pivot_idx = i;
        break;
      }
    }

    if (pivot_idx == -1)
      return clusters;

    // Création du cluster associé à ce pivot
    Cluster c;
    UserPoint u = users[pivot_idx];
    c.id = cluster_id_gen++;
    c.center_lat = users[pivot_idx].lat;
    c.center_lon = users[pivot_idx].lon;
    c.sum_pir = 0.0;
    c.sum_cir = 0.0;
    c.current_load = 0.0;

    for (int i = 0; i < n; ++i) {
      if (!assigned[i]) {
        double dist =
            haversine(c.center_lat, c.center_lon, users[i].lat, users[i].lon);

        if (dist <= RAYON_KM) {
          // Vérifier la charge selon la stratégie
          double charge_ajoutee = 0;
          if (strategie == PESSIMISTE)
            charge_ajoutee = users[i].pir;
          else if (strategie == OPTIMISTE)
            charge_ajoutee = users[i].cir;
          else if (strategie == OVERBOOKING)
            charge_ajoutee = (0.7 * users[i].cir + 0.3 * users[i].pir);

          if (c.current_load + charge_ajoutee <= capacite_cible_mbps) {
            c.users_id.push_back(users[i].id);
            c.current_load += charge_ajoutee;
            c.sum_pir += users[i].pir;
            c.sum_cir += users[i].cir;
            assigned[i] = true;
            assigned_count++;
          }
        }
      }
    }
    clusters.push_back(c);
  }
  if (use_merge) clusters = mergeClusters(clusters , users, capacite_cible_mbps, strategie);
  return clusters;
}

std::vector<Cluster> runQuadtreeClustering(const std::vector<UserPoint> &users,
                                           Strategie_t strategie,
                                           int remplissage,
                                           ShiftStrategy strategy_traitement,
                                           bool global_mean, bool use_hilbert,
                                           bool use_merge) {
  std::vector<Cluster> clusters;
  int n = users.size();
  if (n == 0)
    return clusters;

  Boundary world = {0.0, 0.0, 180.0, 90.0};
  Quadtree tree(world);
  for (const auto &u : users) {
    tree.insert(u);
  }

  std::vector<bool> assigned(n + 1, false);
  int assigned_count = 0;
  int cluster_id_gen = 1;
  double capacite_cible_mbps = (CAP_MAX_GBPS * 1000.0) * (remplissage / 100.0);

  // --- NOUVEAUTÉ : Création et tri des indices ---
  std::vector<int> pivot_indices(n);
  std::iota(pivot_indices.begin(), pivot_indices.end(),
            0); // Remplit avec 0, 1, 2... n-1

  if (use_hilbert) {
    sortIndicesByHilbert(users, pivot_indices);
  }

  // Boucle principale : On itère sur pivot_indices, pas sur users !
  for (int idx : pivot_indices) {
    const auto &pivot = users[idx]; // On récupère le bon pivot

    if (assigned[pivot.id])
      continue;

    Cluster c;
    c.id = cluster_id_gen++;
    c.center_lat = pivot.lat;
    c.center_lon = pivot.lon;
    c.current_load = 0.0;
    c.sum_pir = 0.0;
    c.sum_cir = 0.0;

    double range_deg =
        1.5; // (Si tu as des soucis, tu pourras essayer de passer à 1.0)
    Boundary searchRange = {c.center_lon, c.center_lat, range_deg, range_deg};

    std::vector<UserPoint> candidates;
    tree.queryRange(searchRange, candidates);

    for (const auto &cand : candidates) {
      if (assigned[cand.id])
        continue;

      double dist = haversine(c.center_lat, c.center_lon, cand.lat, cand.lon);

      if (dist <= RAYON_KM) {
        double charge_ajoutee = (strategie == PESSIMISTE) ? cand.pir
                                : (strategie == OPTIMISTE)
                                    ? cand.cir
                                    : (0.7 * cand.cir + 0.3 * cand.pir);

        if (c.current_load + charge_ajoutee <= capacite_cible_mbps) {
          c.users_id.push_back(cand.id);
          c.current_load += charge_ajoutee;
          c.sum_pir += cand.pir;
          c.sum_cir += cand.cir;
          assigned[cand.id] = true;
          assigned_count++; // Méthode delta (O(1))
        }
      }
    }

    if (strategy_traitement != ShiftStrategy::NONE) {
      int count_before = c.users_id.size();
      applyShiftSingle(c, users, assigned, tree, capacite_cible_mbps, strategie,
                       strategy_traitement);
      int count_after = c.users_id.size();
      assigned_count += (count_after - count_before);
    }

    clusters.push_back(c);

    if (assigned_count >= n)
      break;
  }

  if (strategy_traitement == ShiftStrategy::MEAN && global_mean) {
    applyMeanShift(clusters, users, assigned, tree, capacite_cible_mbps,
                   strategie);
  }

  if (use_merge) clusters = mergeClusters(clusters , users, capacite_cible_mbps, strategie);

  return clusters;
}