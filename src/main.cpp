#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include "hilbert.hpp"
#include "metrics.hpp"
#include "shift.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::string path = "../data/generated.csv";
  std::vector<UserPoint> users = CsvToUser(path);

  if (users.empty()) {
    std::cerr << "Erreur : Fichier vide ou introuvable." << std::endl;
    return 1;
  }

  std::vector<Strategie_t> strategies = {PESSIMISTE, OPTIMISTE, OVERBOOKING};
  std::vector<std::string> str_names = {"Pessimiste", "Optimiste",
                                        "Overbooking"};
  std::vector<int> taux_remplissage = {80, 100};

  // Test Hilbert activé et désactivé
  std::vector<bool> hilbert_options = {false, true};

  struct QuadtreeConfig {
    ShiftStrategy strat;
    bool global_mean;
    std::string name;
  };

  std::vector<QuadtreeConfig> q_configs = {
      {ShiftStrategy::NONE, false, "None"},
      {ShiftStrategy::CENTROID, false, "Centroid"},
      {ShiftStrategy::BEST_USER, false, "BestUser"},
      {ShiftStrategy::MEAN, false, "Mean(Local)"},
      {ShiftStrategy::MEAN, true, "Mean(Global)"}};

  std::cout << "\n--- Tableau de Bord du Clustering ---" << std::endl;
  std::cout << std::left << std::setw(10) << "Algo" << std::setw(12)
            << "Stratégie" << std::setw(6) << "Cap%" << std::setw(13)
            << "Post-Trait" << std::setw(8) << "Hilbert" // NOUVELLE COLONNE
            << std::setw(10) << "Clusters" << std::setw(10) << "Rempli(%)"
            << std::setw(13) << "Dist_Moy(km)" << std::setw(10) << "Mem(Ko)"
            << std::setw(10) << "Temps(ms)" << std::endl;
  std::cout << std::string(106, '-') << std::endl;

  // Quadtree Tests
  for (bool use_hilbert : hilbert_options) {
    for (const auto &config : q_configs) {
      for (size_t s = 0; s < strategies.size(); ++s) {
        for (int fill : taux_remplissage) {

          double capacite_cible_mbps = (CAP_MAX_GBPS * 1000.0) * (fill / 100.0);

          auto start = std::chrono::high_resolution_clock::now();
          std::vector<Cluster> clusters =
              runQuadtreeClustering(users, strategies[s], fill, config.strat,
                                    config.global_mean, use_hilbert);
          auto end = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double, std::milli> duration = end - start;

          ClusteringMetrics metrics =
              computeMetrics(clusters, users, capacite_cible_mbps);

          std::string hilbert_str = use_hilbert ? "OUI" : "NON";

          std::cout << std::left << std::setw(10) << "Quadtree" << std::setw(12)
                    << str_names[s] << std::setw(6) << fill << std::setw(13)
                    << config.name << std::setw(8) << hilbert_str
                    << std::setw(10) << metrics.nb_clusters << std::fixed
                    << std::setprecision(2) << std::setw(10)
                    << metrics.avg_fill_rate_percent << std::setw(13)
                    << metrics.avg_distance_km << std::setw(10)
                    << metrics.memory_footprint_kb << std::setw(10)
                    << duration.count() << std::endl;
        }
      }
      std::cout << std::string(106, '-') << std::endl;
    }
    std::cout << std::string(106, '=') << std::endl;
  }

  return 0;
}