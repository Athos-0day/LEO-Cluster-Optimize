#include "list_user_to_tow_list_user.hpp"
#include "csv_read.hpp"
#include <algorithm> // Pour std::shuffle
#include <random>    // Pour std::device et std::mt19937
#include <fstream>
#include <iostream>

void one_to_tow(std::vector<UserPoint> users, std::string fileDataInit, std::string fileDataTest, float proportion) {
    if (users.empty()) {
        return;
    }

    // 1. Mélange aléatoire des utilisateurs
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(users.begin(), users.end(), g);

    // 2. Calcul de l'indice de séparation basé sur la proportion
    // On s'assure que la proportion reste entre 0 et 1
    if (proportion > 1.0f) proportion = 1.0f;
    if (proportion < 0.0f) proportion = 0.0f;

    size_t splitIndex = static_cast<size_t>(users.size() * proportion);

    // 3. Écriture du premier fichier (Initialisation des clusters)
    std::ofstream fileInit(fileDataInit);
    if (fileInit.is_open()) {
        for (size_t i = 0; i < splitIndex; ++i) {
            // Ici, on suppose que UserPoint a une méthode ou des attributs exportables
            // Exemple générique : id, x, y
            fileInit << users[i].id << "," 
                  << users[i].lon << ","
                  << users[i].lat << ","
                  << users[i].pir << ","
                  << users[i].cir << ","
                  << users[i].service << "\n";
        }
        fileInit.close();
    } else {
        std::cerr << "Erreur : Impossible d'ouvrir " << fileDataInit << std::endl;
    }

    // 4. Écriture du second fichier (Données de test / ajout)
    std::ofstream fileTest(fileDataTest);
    if (fileTest.is_open()) {
        for (size_t i = splitIndex; i < users.size(); ++i) {
            fileTest << users[i].id << "," 
                  << users[i].lon << ","
                  << users[i].lat << ","
                  << users[i].pir << ","
                  << users[i].cir << ","
                  << users[i].service << "\n";
        }
        fileTest.close();
    } else {
        std::cerr << "Erreur : Impossible d'ouvrir " << fileDataTest << std::endl;
    }
}