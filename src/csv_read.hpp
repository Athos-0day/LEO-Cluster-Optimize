/**
 * @file csv_read.hpp
 * @author Arthur
 * @brief Permet de lire un fichier csv pour le transformer en une structure
 * lisible.
 * @version 0.1
 * @date 2026-03-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef CSV_READ_HPP
#define CSV_READ_HPP

#include <string>
#include <vector>

/**
 * @brief La structure associé à un utilisateur.
 *
 */
struct UserPoint {
  int id;
  double lat;
  double lon;
  double cir;
  double pir;
  double service;
};

/**
 * @brief Permet de lire le csv pour le transformer en des données utilisables
 * @param filename le nom du fichier.
 * @return les données sous forme de la structure UserPoint.
 *
 */
std::vector<UserPoint> CsvToUser(const std::string &filename);

#endif // CSV_READ_HPP