/**
 * @file geo_utils.hpp
 * @author Arthur
 * @brief Outils de calcul géodésique pour le clustering de satellites.
 * @version 0.1
 * @date 2026-03-20
 */

#ifndef GEO_UTILS_HPP
#define GEO_UTILS_HPP

#include <cmath>

/**
 * @brief Rayon moyen de la Terre en kilomètres.
 */
#define EARTH_RADIUS_KM 6371.0

/**
 * @brief Calcule la distance entre deux coordonnées GPS en utilisant la formule
 * de Haversine.
 * * @param lat1 Latitude du premier point (en degrés décimaux).
 * @param lon1 Longitude du premier point (en degrés décimaux).
 * @param lat2 Latitude du deuxième point (en degrés décimaux).
 * @param lon2 Longitude du deuxième point (en degrés décimaux).
 * @return double La distance en kilomètres entre les deux points.
 */
double haversine(double lat1, double lon1, double lat2, double lon2);

/**
 * @brief Convertit des degrés en radians.
 * * @param degree Valeur en degrés.
 * @return double Valeur en radians.
 */
inline double toRadians(double degree) { return degree * (M_PI / 180.0); }

#endif // GEO_UTILS_HPP