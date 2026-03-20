#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

/**
 * @brief Structure pour stocker les résultats d'un test
 */
struct BenchResult {
  std::string strategie;
  int remplissage;
  int nb_clusters;
  double temps_ms;
};

int main() {
  std::string path = "../data/generated.csv";
  std::vector<UserPoint> users = CsvToUser(path);

  if (users.empty())
    return 1;

  std::vector<Strategie_t> strategies = {PESSIMISTE, OPTIMISTE, OVERBOOKING};
  std::vector<std::string> str_names = {"Pessimiste", "Optimiste",
                                        "Overbooking"};
  std::vector<int> taux_remplissage = {80, 90, 100};

  std::vector<BenchResult> results;

  std::cout << "--- Demarrage du Benchmark de Clustering ---" << std::endl;
  std::cout << std::left << std::setw(15) << "Strat" << std::setw(10) << "Fill%"
            << std::setw(15) << "Clusters" << std::setw(15) << "Temps (ms)"
            << std::endl;
  std::cout << std::string(55, '-') << std::endl;

  for (size_t s = 0; s < strategies.size(); ++s) {
    for (int fill : taux_remplissage) {

      // Mesure du temps
      auto start = std::chrono::high_resolution_clock::now();

      std::vector<Cluster> clusters =
          runGreedyClustering(users, strategies[s], fill);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> duration = end - start;

      // Stockage et affichage
      std::cout << std::left << std::setw(15) << str_names[s] << std::setw(10)
                << fill << std::setw(15) << clusters.size() << std::setw(15)
                << duration.count() << std::endl;

      results.push_back(
          {str_names[s], fill, (int)clusters.size(), duration.count()});
    }
    std::cout << std::string(55, '-') << std::endl;
  }

  return 0;
}