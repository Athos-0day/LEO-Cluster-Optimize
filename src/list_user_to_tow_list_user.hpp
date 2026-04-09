/**
 * @file geo_utils.hpp
 * @author Louan
 * @brief Permet de prendre une liste d'utilisateur et de la séparer en 2 liste pour des données initial et des utilisateurs qui regoindront ensuite.
 * @version 0.1
 * @date 2026-03-20
 */

#ifndef LIST_USER_TO_TOW_LIST_USER_HPP
#define LIST_USER_TO_TOW_LIST_USER_HPP

#include <string>
#include <vector>
#include "csv_read.hpp"

/**
 * @brief Permet de lire le csv pour le transformer en des données utilisables
 * @param users liste complete des utilisateurs.
 * @param fileDataInit nom du fichier pour les utilisateurs qui serviront a faire les premiers clusters
 * @param fileDataTest nom du fichier pour les utilisateurs qui seront rajouter au clusters.
 * @param proportion proportion d'utilisateur dans le prmier fichier
 * 
 * @return les données sous forme fichier csv contant les users
 *
 */
void one_to_tow(std::vector<UserPoint> users, std::string fileDataInit, std::string fileDataTest, float proportion);

#endif
