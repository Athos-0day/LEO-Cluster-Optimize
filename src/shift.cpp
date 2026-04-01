#include "shift.hpp"
#include "geo_utils.hpp"
#include <cmath>

void applyShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                      std::vector<bool> &assigned, const Quadtree &tree,
                      double capacite_cible_mbps, Strategie_t strategie_qos,
                      ShiftStrategy strategie_shift) {

  if (strategie_shift == ShiftStrategy::CENTROID) {
    applyCentroidShiftSingle(c, users, assigned, tree, capacite_cible_mbps,
                             strategie_qos);
  } else if (strategie_shift == ShiftStrategy::BEST_USER) {
    applyBestUserShiftSingle(c, users, assigned, tree, capacite_cible_mbps,
                             strategie_qos);
  } else if (strategie_shift == ShiftStrategy::MEAN) {
    applyMeanShiftSingle(c, users, assigned, tree, capacite_cible_mbps,
                         strategie_qos);
  }
  // ShiftStrategy::NONE ne fait rien, le centre reste inchangé
}

void applyCentroidShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                              std::vector<bool> &assigned, const Quadtree &tree,
                              double capacite_cible_mbps,
                              Strategie_t strategie_qos) {
  // Calcul du barycentre
  double sum_lat = 0.0, sum_lon = 0.0;
  int count = 0;
  for (int uid : c.users_id) {
    const auto &u = users[uid - 1];
    sum_lat += u.lat;
    sum_lon += u.lon;
    count++;
  }
  if (count > 0) {
    c.center_lat = sum_lat / count;
    c.center_lon = sum_lon / count;
  }
}

void applyBestUserShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                              std::vector<bool> &assigned, const Quadtree &tree,
                              double capacite_cible_mbps,
                              Strategie_t strategie) {
  // Trouver l'utilisateur le plus critique selon la stratégie QoS
  int best_uid = -1;
  double worst_metric = -1.0;
  for (int uid : c.users_id) {
    const auto &u = users[uid - 1];
    double metric = (strategie == Strategie_t::PESSIMISTE) ? u.pir
                    : (strategie == Strategie_t::OPTIMISTE)
                        ? u.cir
                        : 0.7 * u.cir + 0.3 * u.pir; // OVERBOOKING
    if (metric > worst_metric) {
      worst_metric = metric;
      best_uid = uid;
    }
  }
  if (best_uid != -1) {
    c.center_lat = users[best_uid - 1].lat;
    c.center_lon = users[best_uid - 1].lon;
  }
}

/**
 * @brief Calcule la charge d'un utilisateur selon la stratégie QoS choisie.
 */
static double getUserCharge(const UserPoint &u, Strategie_t strategie) {
  if (strategie == PESSIMISTE)
    return u.pir;
  if (strategie == OPTIMISTE)
    return u.cir;
  return (0.7 * u.cir + 0.3 * u.pir); // OVERBOOKING
}

void applyMeanShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                          std::vector<bool> &assigned, const Quadtree &tree,
                          double capacite_cible_mbps,
                          Strategie_t strategie_qos) {

  bool a_bouge = true;
  int max_iterations = 5;
  int iter = 0;
  const double SEUIL_MOUVEMENT_KM = 0.05; // 50 mètres

  while (a_bouge && iter < max_iterations) {
    double old_lat = c.center_lat;
    double old_lon = c.center_lon;

    // --- ÉTAPE 1 : Calcul du nouveau barycentre ---
    if (!c.users_id.empty()) {
      double sum_lat = 0.0, sum_lon = 0.0;
      for (int id : c.users_id) {
        const auto &u =
            users[id - 1]; // Attention: id-1 si les IDs commencent à 1
        sum_lat += u.lat;
        sum_lon += u.lon;
      }
      c.center_lat = sum_lat / c.users_id.size();
      c.center_lon = sum_lon / c.users_id.size();
    }

    // --- ÉTAPE 2 : Relâchement (Release) ---
    // On vire les utilisateurs qui sont désormais trop loin du nouveau centre
    std::vector<int> kept_users;
    double new_load = 0.0, new_sum_pir = 0.0, new_sum_cir = 0.0;

    for (int id : c.users_id) {
      const auto &u = users[id - 1];
      double dist = haversine(c.center_lat, c.center_lon, u.lat, u.lon);

      if (dist <= RAYON_KM) {
        // Ils sont toujours dans le cercle, on les garde
        kept_users.push_back(id);
        double charge = getUserCharge(u, strategie_qos);
        new_load += charge;
        new_sum_pir += u.pir;
        new_sum_cir += u.cir;
      } else {
        // Ils sont sortis du faisceau à cause du mouvement ! On les libère.
        assigned[id] = false;
      }
    }
    // Mise à jour propre du cluster avant de recapturer
    c.users_id = kept_users;
    c.current_load = new_load;
    c.sum_pir = new_sum_pir;
    c.sum_cir = new_sum_cir;

    // --- ÉTAPE 3 : Re-capture (Aspiration) ---
    Boundary searchRange = {c.center_lon, c.center_lat, 0.6, 0.6};
    std::vector<UserPoint> candidates;
    tree.queryRange(searchRange, candidates);

    bool nouveaux_points_ajoutes = false;
    for (const auto &cand : candidates) {
      if (assigned[cand.id])
        continue;

      double dist = haversine(c.center_lat, c.center_lon, cand.lat, cand.lon);
      if (dist <= RAYON_KM) {
        double charge = getUserCharge(cand, strategie_qos);

        if (c.current_load + charge <= capacite_cible_mbps) {
          c.users_id.push_back(cand.id);
          c.current_load += charge;
          c.sum_pir += cand.pir;
          c.sum_cir += cand.cir;
          assigned[cand.id] = true;
          nouveaux_points_ajoutes = true;
        }
      }
    }

    // --- ÉTAPE 4 : Convergence ---
    double deplacement =
        haversine(old_lat, old_lon, c.center_lat, c.center_lon);
    a_bouge = (deplacement > SEUIL_MOUVEMENT_KM) || nouveaux_points_ajoutes;
    iter++;
  }
}

void applyMeanShift(std::vector<Cluster> &clusters,
                    const std::vector<UserPoint> &users,
                    std::vector<bool> &assigned, const Quadtree &tree,
                    double capacite_cible_mbps, Strategie_t strategie_qos) {

  // On applique l'optimisation itérative à chaque cluster de la solution
  for (auto &c : clusters) {
    applyMeanShiftSingle(c, users, assigned, tree, capacite_cible_mbps,
                         strategie_qos);
  }
}