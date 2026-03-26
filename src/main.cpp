#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include "metrics.hpp"
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

  struct Algo {
    std::string name;
    std::vector<Cluster> (*func)(const std::vector<UserPoint> &, Strategie_t,
                                 int);
  };

  std::vector<Algo> algos = {{"Classique", runGreedyClustering},
                             {"Quadtree", runQuadtreeClustering}};

  std::cout << "\n--- Tableau de Bord du Clustering ---" << std::endl;
  std::cout << std::left << std::setw(10) << "Algo" << std::setw(12)
            << "Stratégie" << std::setw(6) << "Cap%" << std::setw(10)
            << "Clusters" << std::setw(12) << "Rempli(%)" << std::setw(12)
            << "Dist_Moy(km) " << std::setw(12) << "Mem(Ko)" << std::setw(12)
            << "Temps(ms)" << std::endl;
  std::cout << std::string(86, '-') << std::endl;

  for (const auto &algo : algos) {
    for (size_t s = 0; s < strategies.size(); ++s) {
      for (int fill : taux_remplissage) {

        double capacite_cible_mbps = (CAP_MAX_GBPS * 1000.0) * (fill / 100.0);

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<Cluster> clusters = algo.func(users, strategies[s], fill);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;

        // Calcul de nos nouvelles métriques
        ClusteringMetrics metrics =
            computeMetrics(clusters, users, capacite_cible_mbps);

        std::cout << std::left << std::setw(10) << algo.name << std::setw(12)
                  << str_names[s] << std::setw(6) << fill << std::setw(10)
                  << metrics.nb_clusters << std::fixed << std::setprecision(2)
                  << std::setw(12) << metrics.avg_fill_rate_percent
                  << std::setw(12) << metrics.avg_distance_km << std::setw(12)
                  << metrics.memory_footprint_kb << std::setw(12)
                  << duration.count() << std::endl;
      }
    }
    std::cout << std::string(86, '=') << std::endl;
  }

  return 0;
}