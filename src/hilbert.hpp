/**
 * @file hilbert.hpp
 * @author Arthur
 * @brief Permet de classer les users sur la courbe de hilbert.
 * @version 0.1
 * @date 2026-04-05
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef HILBERT_HPP
#define HILBERT_HPP

#include "csv_read.hpp"
#include <vector>

// Trie un vecteur d'indices (0 à N-1) selon la position sur la courbe de
// Hilbert
void sortIndicesByHilbert(const std::vector<UserPoint> &users,
                          std::vector<int> &indices);

#endif // HILBERT_HPP