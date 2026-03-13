# Projet ingénierie des réseaux (2025-2026)

## Sujet : Clustering de terminaux utilisateurs dans un réseau de constellations de satellites

On souhaite construire des **clusters d'utilisateurs** afin de faire pointer une constellation de satellite de façon optimale. Les satellites sont du types LEO (Low Earth Orbit), ils servent notamment à fournir un accès au réseau type *Starlink*. Ces satellites ont généralement une période de 128 minutes (soit au minilul 11 périodes par jour). Ainsi, il est essentiel de prendre en considération le déplacement constant des satellites dans ce type de constellation. En effet, au bout d'une certaine période, il faudra de nouveau orienter le faisceau des satellites. Concrètement, il faut exécuter les algorithmes de manière régulière (centralisée ou décentralisée). En conséquence, notre solution doit être un compromis entre rapidité afin de calculer les nouveaux clusters et précision afin de d'optimiser le nombre de clusters.

Il est nécessaire de minimiser les clusters afin de réduire le nombre de faisceaux projetés par les satellites et ainsi réduire les coûts. Cependant, il faut garder une marge afin de ne pas être à 100% de charge et permettre la flexibilité pour les nouveaux utilisateurs.
Pour ce projet, la difficulté principale est de trouver la meilleur pipeline de traitement possible afin de prendre en compte l'efficacité permettant le passage à l'échelle, les coûts, ainsi que la qualité de service.

Nous possédons les localisations des utlisateurs au format **CSV/HTML**.

Hypothèses :
- Cluster de taille fixe (**90km**): chaque cluster aura un diamètre de 90km.
- Pas de contrainte sur le centre du cluster: pas besoin de pointer le centre sur des endroits précis.
- Plusieurs clusters peuvent se superposer: permet de répondre à un besoin important dans une zone spécifique.
- Débit par cluster: 1, 2 ou 3 Gps.

## Ordre de traitement

La première question est de se demander comment nous allons traiter les informations, i.e faire un prétraitement des données, les prendre aléatoirement... Pour cela, on va d'abord faire un état de l'art.

### Propositions précédentes des étudiants de l'ENSEEIHT

L'idée du groupe précédant est de réaliser un traitement en trois phases:
- une phase de prétraitement afin d'éviter de traiter les données comme un bloc monolithique lors de la phase de recherche de clusters (clustering). Il est question de répartir les données en sous-groupe dans le but d'accroître la performance de l'algorithme de clutering et de répondre au critère technique de la performance. 

Au départ 87 898 utilisateurs initiaux, l'algorithme génère 4 169 zones connexes.

Cette approche est clairement pertinente afin de réduire le nombre de données à traiter lors de la recherche des clusters afin d'augmenter la performance de l'algorithme. En effet, ce traitement ne se fait qu'à chaque fois qu'il y a un changement dans la base de données et encore on pourrait adapter l'algorithme pour qu'il ne recalcule que les zones concernés mais cela n'est pas l'objectif. On considère une base de données **statiques**. Traitement monolithique implique une précision maximale pour les clusters mais nécessite plus de temps.

### État de l'art : Clustering Strategies in Satellite-Aided Communications (2025)

[Article Link](https://arxiv.org/pdf/2509.13701)

Ce papier analyse l'arbitrage critique entre la **vitesse d'exécution** et l'**optimisation du réseau** pour les satellites LEO.

#### Points clés retenus pour le projet :
* **Validation de la rapidité :** L'usage d'algorithmes de théorie des graphes (type zones connexes/Greedy) est indispensable pour traiter les ~88k utilisateurs en quelques secondes.
* **Gestion de la densité :** Utilisation de logiques type **DBSCAN** pour gérer les zones nécessitant des clusters superposés (jusqu'à 4 Gbps).
* **Réduction des interférences :** Application de la **coloration de graphe** pour séparer les fréquences des clusters qui se chevauchent.
* **Optimisation post-calcul :** Les algorithmes génétiques ou le recentrage par centroïdes (K-Means) permettent d'affiner la précision après le prétraitement rapide.

### État de l'art : Analysis of the Clustering Properties of the Hilbert Space-Filling Curve (2001)

Ce papier démontre l'efficacité de **la courbe de Hilbert** pour les problèmes de clustering. Il étduie les propriétés de clustering de la courbe de remplissage d'espace de Hilbert. Ces courbes permettent de passer d'un espace multidimensionnel à un espace d'une dimension.

#### Points clés retenus pour le projet :

* Les objets proches dans l'espace multidimensionnel sont proches dans l'espace de dimension 1 (Préservation de la localité spatiale).
* L'article démontre mathématiquement et empiriquement que la courbe de Hilbert surpasse ses concurrentes
* Si quelques utilisateurs bougent légèrement, leur indice de Hilbert change peu.
* Alors que des algorithmes comme DBSCAN ou K-Means sont coûteux en calcul, le "Hilbert-Clustering" est d'une rapidité extrême.
* Chaque utilisateur se voit attribuer un "Hilbert Index" unique. C'est ici que la magie opère : si deux utilisateurs sont dans la même zone géographique de 90 km, leurs indices de Hilbert seront très proches numériquement.
* On trie la liste des utilisateurs par ordre croissant d'index.

### État de l'art : Survey of Clustering Algorithms (2005)

[Article Link](https://ieeexplore-ieee-org.gorgone.univ-toulouse.fr/stampPDF/getPDF.jsp?tp=&arnumber=1427769&ref=aHR0cHM6Ly9pZWVleHBsb3JlLmllZWUub3JnL2RvY3VtZW50LzE0Mjc3Njk=&tag=1)

Ce papier explique ce qu'est le clustering et présente les algorithmes existants.

### Etat de l'art : Algorithms for Disk Covering Problems with the Most Points
Ce papier traîte des d'un nouvel algorithme glouton a heurstique permettant un clustering efficace selon nos contrainte.

* Ces avantages: cette méthode donne pour un nombre de point non complet une distribution optimale des clusters (pour une taille de cluster fixe), et une très bonne approxiamation pour le nombre complet de point.
* Ces défaut: malheureusement, l'approche gloutonne est très demandante en ressource (complexité élevée $O(K*N^2)$).
* Extension: on peut combiner cette méthode avec d'autre méthode afin d'améliorer sa complexité notamment en utilisant des heuristiques plus efficaces (mean shift,...)

#### Points clés retenus pour le projet :

* Algorithmes Partitionnels (Centroïdes) :
    + K-means : Très efficace avec une complexité de $O(NKd)$. C'est le standard pour les gros jeux de données, bien qu'il puisse converger vers des optima locaux.
    + PAM (K-medoids) : Plus robuste aux bruits mais plus coûteux, avec une complexité de $O(K(N-K)^2)$.

* Algorithmes Hiérarchiques :
    + Méthodes classiques : Généralement très gourmandes, avec des complexités allant de $O(N^2)$ à $O(N^3)$, ce qui les limite aux petits jeux de données.
    + BIRCH : Optimisé pour le Big Data grâce à un arbre de caractéristiques, ramenant la complexité à $O(N)$.
    + CURE : Utilise l'échantillonnage pour atteindre $O(N_{s}^2 \log N_{s})$ (où $N_s$ est la taille de l'échantillon).
* Algorithmes Basés sur la Densité et les Grilles :
    + DBSCAN : Capable de trouver des formes arbitraires. Sa complexité est de $O(N \log N)$ avec des structures spatiales (type R*-tree), mais monte à $O(N^2)$ dans le pire des cas.
    + CLIQUE : Combine densité et grilles pour la haute dimension avec une complexité de $O(N + \text{nombre de cellules})$.
