///////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes for abiotic conditions for herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date November 2017
////////////////////////////////////////////////////////////////////
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

namespace Fauna {
/// Variables describing the habitat that are not forage ⇒ abiotic environment.
struct HabitatEnvironment {
  /// Constructor with default values.
  HabitatEnvironment() : air_temperature(20.0), snow_depth(0.0) {}

  /// Ambient air temperature near ground [°C], whole-day average.
  double air_temperature;

  /// Depth of snow cover [cm].
  double snow_depth;
};
}  // namespace Fauna

#endif  // ENVIRONMENT_H
