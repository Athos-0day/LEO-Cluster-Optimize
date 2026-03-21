#include "greedy_disk_clustering.hpp"
#include "geo_utils.hpp"
#include "quadtree.hpp"

std::vector<Cluster> runGreedyClustering(const std::vector<UserPoint> &users,
                                         Strategie_t strategie,
                                         int remplissage) {
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

  return clusters;
}

std::vector<Cluster> runQuadtreeClustering(const std::vector<UserPoint> &users,
                                           Strategie_t strategie,
                                           int remplissage) {
  std::vector<Cluster> clusters;
  int n = users.size();
  if (n == 0)
    return clusters;

  // 1. Initialisation de l'arbre spatial
  // On définit une zone englobante (Boundary) couvrant les données GPS
  Boundary world = {0.0, 0.0, 180.0, 90.0};
  Quadtree tree(world);
  for (const auto &u : users) {
    tree.insert(u);
  }

  std::vector<bool> assigned(n + 1, false); // +1 car tes IDs commencent à 1
  int assigned_count = 0;
  int cluster_id_gen = 1;
  double capacite_cible_mbps = (CAP_MAX_GBPS * 1000.0) * (remplissage / 100.0);

  // 2. Boucle principale
  for (const auto &pivot : users) {
    if (assigned[pivot.id])
      continue;

    Cluster c;
    c.id = cluster_id_gen++;
    c.center_lat = pivot.lat;
    c.center_lon = pivot.lon;
    c.current_load = 0.0;
    c.sum_pir = 0.0;
    c.sum_cir = 0.0;

    // 3. Recherche spatiale ciblée
    // 45km correspondent à environ 0.45 degré (approximation sécurisée)
    double range_deg = 0.5;
    Boundary searchRange = {c.center_lon, c.center_lat, range_deg, range_deg};

    std::vector<UserPoint> candidates;
    tree.queryRange(searchRange, candidates);

    // 4. Remplissage parmis les candidats proches uniquement
    for (const auto &cand : candidates) {
      if (assigned[cand.id])
        continue;

      double dist = haversine(c.center_lat, c.center_lon, cand.lat, cand.lon);

      if (dist <= RAYON_KM) {
        double charge_ajoutee = 0;
        if (strategie == PESSIMISTE)
          charge_ajoutee = cand.pir;
        else if (strategie == OPTIMISTE)
          charge_ajoutee = cand.cir;
        else if (strategie == OVERBOOKING)
          charge_ajoutee = (0.7 * cand.cir + 0.3 * cand.pir);

        if (c.current_load + charge_ajoutee <= capacite_cible_mbps) {
          c.users_id.push_back(cand.id);
          c.current_load += charge_ajoutee;
          c.sum_pir += cand.pir;
          c.sum_cir += cand.cir;
          assigned[cand.id] = true;
          assigned_count++;
        }
      }
    }
    clusters.push_back(c);

    if (assigned_count >= n)
      break;
  }

  return clusters;
}