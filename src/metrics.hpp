/**
 * @file metrics.hpp
 * @author Arthur
 * @brief Outils d'évaluation et de calcul de métriques pour le clustering.
 * @version 0.1
 */

#ifndef METRICS_HPP
#define METRICS_HPP

#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include <vector>

/**
 * @brief Structure contenant les métriques de qualité et de performance du
 * clustering.
 */
struct ClusteringMetrics {
  int nb_clusters;              ///< Nombre total de clusters générés
  double avg_fill_rate_percent; ///< Taux de remplissage moyen des clusters (%)
  double avg_distance_km;       ///< Distance moyenne entre un utilisateur et le
                                ///< centre de son cluster (km)
  double memory_footprint_kb; ///< Estimation de la mémoire allouée pour stocker
                              ///< les résultats (Ko)
};

/**
 * @brief Calcule les métriques d'évaluation pour un ensemble de clusters.
 * * @param clusters Le vecteur de clusters généré par l'algorithme.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param capacite_cible_mbps La capacité maximale autorisée d'un cluster en
 * Mbps.
 * @return ClusteringMetrics Les métriques calculées.
 */
ClusteringMetrics computeMetrics(const std::vector<Cluster> &clusters,
                                 const std::vector<UserPoint> &users,
                                 double capacite_cible_mbps);

#endif // METRICS_HPP