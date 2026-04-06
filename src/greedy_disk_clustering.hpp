/**
 * @file greedy_disk_clustering.hpp
 * @author Arthur
 * @brief
 * @version 0.1
 * @date 2026-03-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef GREEDY_DISK_CLUSTERING_HPP
#define GREEDY_DISK_CLUSTERING_HPP

#include "csv_read.hpp"
#include "hilbert.hpp"
#include "quadtree.hpp"
#include <vector>

#define CAP_MAX_GBPS 3.0
#define RAYON_KM 45.0

/**
 * @brief Enumeration des stratégies de clustering
 *
 */
enum Strategie_t {
  PESSIMISTE, // Somme des PIR <= Capacité du cluster
  OPTIMISTE,  // Somme des CIR <= Capacité du cluster
  OVERBOOKING // Somme alpha CIR + beta PIR <= Capacité du cluster (alpha + beta
              // = 1)
};

enum class ShiftStrategy;

/**
 * @brief Structure pour stocker les données d'un cluster.
 *
 */
struct Cluster {
  int id;
  int capacite_cluster;
  double center_lon;
  double center_lat;
  double current_load;       // La charge calculée selon la stratégie choisie
  double sum_pir;            // Somme des pir
  double sum_cir;            // Somme des cir
  std::vector<int> users_id; // Id des utilisateurs dans le cluster
};

/**
 * @brief Permet de construire les clusters avec l'algorithme glouton.
 * * L'algorithme assigne chaque utilisateur à un cluster de 90km de diamètre
 * en veillant à ne pas dépasser (CAPACITE_MAX * remplissage / 100).
 *
 * @param users Liste des points utilisateurs chargés depuis le CSV.
 * @param strategie Choix de la métrique de charge (PIR, CIR ou mixte).
 * @param remplissage Pourcentage max de charge cible (ex: 80 pour 80%).
 * @return std::vector<Cluster> Vecteur de clusters optimisés.
 */
std::vector<Cluster> runGreedyClustering(const std::vector<UserPoint> &users,
                                         Strategie_t strategie,
                                         int remplissage);

/**
 * @brief Algorithme de clustering optimisé par Quadtree.
 * * Cette version utilise une structure spatiale pour limiter la recherche des
 * voisins aux zones adjacentes. La complexité passe de O(N*K) à O(N log N).
 * * @param users Liste des points utilisateurs.
 * @param strategie Choix de la métrique de charge.
 * @param remplissage Pourcentage max de charge cible.
 * @param strategy_traitement Choix de la stratégie de traitement.
 * @param global_mean Permet de lancer un posttraitement global.
 * @param hilbert permet de trier spatialement les données sur la courbe de
 * hilbert.
 * @return std::vector<Cluster> Vecteur de clusters optimisés.
 */
std::vector<Cluster> runQuadtreeClustering(const std::vector<UserPoint> &users,
                                           Strategie_t strategie,
                                           int remplissage,
                                           ShiftStrategy strategy_traitement,
                                           bool global_mean, bool hilbert);

#endif // GREEDY_DISK_CLUSTERING_HPP