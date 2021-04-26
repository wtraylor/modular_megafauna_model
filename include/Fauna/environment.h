// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Classes for abiotic conditions for herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_ENVIRONMENT_H
#define FAUNA_ENVIRONMENT_H

namespace Fauna {
/// Variables describing the habitat that are not forage ⇒ abiotic environment.
struct HabitatEnvironment {
  /// Ambient air temperature near ground [°C], whole-day average.
  double air_temperature = 20.0;
};
}  // namespace Fauna

#endif  // FAUNA_ENVIRONMENT_H
