#include "greedy_disk_clustering.hpp"
#include "geo_utils.hpp"

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