# Projet ingénierie des réseaux (2025-2026)

## Sujet : Clustering de terminaux utilisateurs dans un réseau de constellations de satellites

On souhaite construire des **clusters d'utilisateurs** afin de faire pointer une constellation de satellite de façon optimale. Les satellites sont du types LEO (Low Earth Orbit), ils servent notamment à fournir un accès au réseau type *Starlink*. Ainsi, il est essentiel de prendre en considération le déplacement constant des satellites dans ce type de constellation. En effet, au bout d'une certaine période, il faudra de nouveau orienter le faisceau des satellites. En conséquence, notre solution doit être un compromis entre rapidité afin de calculer les nouveaux clusters et précision afin de minimiser le nombre de clusters.

Nous possédons les localisations des utlisateurs au format **CSV/HTML**, il faudra donc déveloper une méthode de parsing.

Hypothèses :
- Cluster de taille fixe (**90km**): chaque cluster aura un diamètre de 90km.
- Pas de contrainte sur le centre du cluster: pas besoin de pointer le centre sur des endroits précis.
- Plusieurs clusters peuvent se superposer: permet de répondre à un besoin important dans une zone spécifique.
- Débit par cluster: 1, 2 ou 3 Gps.

## Ordre de traitement

La première question est de se demander comment nous allons traiter les informations, i.e faire un prétraitement des données, les prendre aléatoirement... Pour cela, on va d'abord faire un état de l'art.

### Propositions précédentes des étudiants de l'ENSEEIHT

Leur idée et de faire une phase de prétraitement afin d'éviter de traiter les données comme un bloc monolithique lors de la phase de recherche de clusters. Leur idée est de répartir la liste de données en sous-groupe dans l'idée d'accroître la performance de l'algorithme de clusters et de répondre au critère technique de la performance. Pour cela, ils ont crée des groupes de points proches en utilisant la distance cartésienne. 

Au départ 87 898 utilisateurs initiaux, l'algorithme génère 4 169 zones connexes.

-> L'idée du prétraitement est clairement pertinente afin de réduire le nombre de données à traiter lors de la recherche des clusters afin d'augmenter la performance de l'algorithme. En effet, ce traitement ne se fait qu'à chaque fois qu'il y a un changement dans la base de données et encore on pourrait adapter l'algorithme pour qu'il ne recalcule que les zones concernés mais cela n'est pas l'objectif. On considère une base de données **statiques**. Traitement monolithique implique une précision maximale pour les clusters mais nécessite plus de temps.

### État de l'art : Clustering Strategies in Satellite-Aided Communications (2025)

[Article Link](https://arxiv.org/pdf/2509.13701)

Ce papier analyse l'arbitrage critique entre la **vitesse d'exécution** et l'**optimisation du réseau** pour les satellites LEO.

#### Points clés retenus pour le projet :
* **Validation de la rapidité :** L'usage d'algorithmes de théorie des graphes (type zones connexes/Greedy) est indispensable pour traiter les ~88k utilisateurs en quelques secondes.
* **Gestion de la densité :** Utilisation de logiques type **DBSCAN** pour gérer les zones nécessitant des clusters superposés (jusqu'à 4 Gbps).
* **Réduction des interférences :** Application de la **coloration de graphe** pour séparer les fréquences des clusters qui se chevauchent.
* **Optimisation post-calcul :** Les algorithmes génétiques ou le recentrage par centroïdes (K-Means) permettent d'affiner la précision après le prétraitement rapide.

### État de l'art : Analysis of the Clustering Properties of the Hilbert Space-Filling Curve (2001)

[Article Link](https://www.youtube.com/watch?v=E4WlUXrJgy4)

Ce papier démontre l'efficacité de **la courbe de Hilbert** pour les problèmes de clustering. Il étduie les propriétés de clustering de la courbe de remplissage d'espace de Hilbert. Ces courbes permettent de passer d'un espace multidimensionnel à un espace d'une dimension.

#### Points clés retenus pour le projet :

* Les objets proches dans l'espace multidimensionnel sont proches dans l'espace de dimension 1 (Préservation de la localité spatiale).
* L'article démontre mathématiquement et empiriquement que la courbe de Hilbert surpasse ses concurrentes
* Si quelques utilisateurs bougent légèrement, leur indice de Hilbert change peu.
* Alors que des algorithmes comme DBSCAN ou K-Means sont coûteux en calcul, le "Hilbert-Clustering" est d'une rapidité extrême.
* Chaque utilisateur se voit attribuer un "Hilbert Index" unique. C'est ici que la magie opère : si deux utilisateurs sont dans la même zone géographique de 90 km, leurs indices de Hilbert seront très proches numériquement.
* On trie la liste des utilisateurs par ordre croissant d'index.