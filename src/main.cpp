#include "csv_read.hpp"
#include "greedy_disk_clustering.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

struct BenchResult {
  std::string algo;
  std::string strategie;
  int remplissage;
  int nb_clusters;
  double temps_ms;
};

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
  std::vector<int> taux_remplissage = {
      80, 100}; // On limite pour pas que le main dure 2 heures

  // On définit nos deux fonctions à tester
  struct Algo {
    std::string name;
    std::vector<Cluster> (*func)(const std::vector<UserPoint> &, Strategie_t,
                                 int);
  };
  std::vector<Algo> algos = {{"Classique", runGreedyClustering},
                             {"Quadtree ", runQuadtreeClustering}};

  std::cout << "--- Duel : Greedy Classique vs Quadtree ---" << std::endl;
  std::cout << std::left << std::setw(12) << "Algo" << std::setw(15) << "Strat"
            << std::setw(10) << "Fill%" << std::setw(12) << "Clusters"
            << std::setw(15) << "Temps (ms)" << std::endl;
  std::cout << std::string(65, '-') << std::endl;

  for (const auto &algo : algos) {
    for (size_t s = 0; s < strategies.size(); ++s) {
      for (int fill : taux_remplissage) {

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<Cluster> clusters = algo.func(users, strategies[s], fill);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;

        std::cout << std::left << std::setw(12) << algo.name << std::setw(15)
                  << str_names[s] << std::setw(10) << fill << std::setw(12)
                  << clusters.size() << std::setw(15) << std::fixed
                  << std::setprecision(2) << duration.count() << std::endl;
      }
    }
    std::cout << std::string(65, '=') << std::endl;
  }

  return 0;
}