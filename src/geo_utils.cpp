#include "geo_utils.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double haversine(double lat1, double lon1, double lat2, double lon2) {
  // Conversion des degrés en radians
  double phi1 = toRadians(lat1);
  double phi2 = toRadians(lat2);
  double deltaPhi = toRadians(lat2 - lat1);
  double deltaLambda = toRadians(lon2 - lon1);

  // Application de la formule
  double a = std::sin(deltaPhi / 2.0) * std::sin(deltaPhi / 2.0) +
             std::cos(phi1) * std::cos(phi2) * std::sin(deltaLambda / 2.0) *
                 std::sin(deltaLambda / 2.0);

  double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

  return EARTH_RADIUS_KM * c;
}