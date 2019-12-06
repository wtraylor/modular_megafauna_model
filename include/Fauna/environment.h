/**
 * \file
 * \brief Classes for abiotic conditions for herbivores.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_ENVIRONMENT_H
#define FAUNA_ENVIRONMENT_H

namespace Fauna {
/// Variables describing the habitat that are not forage ⇒ abiotic environment.
struct HabitatEnvironment {
  /// Ambient air temperature near ground [°C], whole-day average.
  double air_temperature = 20.0;

  /// Depth of snow cover [cm].
  double snow_depth = 0.0;
};
}  // namespace Fauna

#endif  // FAUNA_ENVIRONMENT_H
