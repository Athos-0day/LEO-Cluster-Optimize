/**
 * @file metrics.hpp
 * @author Arthur
 * @brief Outils d'évaluation et de calcul de métriques pour le clustering.
 * @version 0.1
 */

#ifndef SHIFT_HPP
#define SHIFT_HPP

#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include "quadtree.hpp"
#include <vector>

/**
 * @enum ShiftStrategy
 * @brief Définit le comportement du repositionnement des faisceaux.
 */
enum class ShiftStrategy {
  NONE,     ///< Statu quo : le centre reste sur le pivot initial.
  CENTROID, ///< Repositionnement au barycentre des membres actuels (Optimise le
            ///< SNR local).
  BEST_USER, ///< Centre déplacé sur l'utilisateur le plus critique (Priorité
             ///< QoS).
  MEAN       ///< Approche itérative : déplace le centre et tente d'agréger de
             ///< nouveaux voisins libres.
};

/**
 * @brief Fonction d'aiguillage pour appliquer une stratégie de shift à un
 * cluster.
 * * @note Cette fonction sert de wrapper pour les implémentations spécifiques.
 * @param cluster Le cluster à déplacer.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param assigned Le masque des utilisateurs déjà assignés.
 * @param tree Le Quadtree pour la re-capture.
 * @param capacite_cible_mbps La limite de charge.
 * @param strategie_qos La stratégie de QoS.
 * @param strategie_shift La stratégie de déplacement du centre.
 */
void applyShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                      std::vector<bool> &assigned, const Quadtree &tree,
                      double capacite_cible_mbps, Strategie_t strategie_qos,
                      ShiftStrategy strategie_shift);

/**
 * @brief Optimise un cluster en appliquant la stratégie
 * de CENTROID.
 * @param cluster Le cluster à optimiser.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param assigned Le masque des utilisateurs déjà assignés.
 * @param tree Le Quadtree pour la re-capture.
 * @param capacite_cible_mbps La limite de charge.
 * @param strategie_qos La stratégie de QoS.
 */
void applyCentroidShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                              std::vector<bool> &assigned, const Quadtree &tree,
                              double capacite_cible_mbps,
                              Strategie_t strategie_qos);

/**
 * @brief Optimise un cluster en appliquant la stratégie
 * de BEST_USER.
 * @param cluster Le cluster à optimiser.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param assigned Le masque des utilisateurs déjà assignés.
 * @param tree Le Quadtree pour la re-capture.
 * @param capacite_cible_mbps La limite de charge.
 * @param strategie_qos La stratégie de QoS.
 */
void applyBestUserShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                              std::vector<bool> &assigned, const Quadtree &tree,
                              double capacite_cible_mbps,
                              Strategie_t strategie);

/**
 * @brief Optimise un cluster en appliquant la stratégie
 * de MEAN.
 * @param cluster Le cluster à optimiser.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param assigned Le masque des utilisateurs déjà assignés.
 * @param tree Le Quadtree pour la re-capture.
 * @param capacite_cible_mbps La limite de charge.
 * @param strategie_qos La stratégie de QoS.
 */
void applyMeanShiftSingle(Cluster &c, const std::vector<UserPoint> &users,
                          std::vector<bool> &assigned, const Quadtree &tree,
                          double capacite_cible_mbps,
                          Strategie_t strategie_qos);

/**
 * @brief Optimise tous les clusters d'une solution en appliquant la stratégie
 * de Mean.
 * @param clusters La liste des clusters à optimiser.
 * @param users La liste originale des utilisateurs (pour retrouver leurs
 * coordonnées).
 * @param assigned Le masque des utilisateurs déjà assignés.
 * @param tree Le Quadtree pour la re-capture.
 * @param capacite_cible_mbps La limite de charge.
 * @param strategie_qos La stratégie de QoS.
 */
void applyMeanShift(Cluster &c, const std::vector<UserPoint> &users,
                    std::vector<bool> &assigned, const Quadtree &tree,
                    double capacite_cible_mbps, Strategie_t strategie_qos);

#endif // SHIFT_HPP