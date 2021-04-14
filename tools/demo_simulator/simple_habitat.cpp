// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Minimal habitat implementation for demonstration purpose.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "simple_habitat.h"
#include <string>

using namespace Fauna;
using namespace Fauna::Demo;

HabitatEnvironment SimpleHabitat::get_environment() const {
  HabitatEnvironment env;
  env.air_temperature = air_temperature;
  return env;
}

void SimpleHabitat::init_day(const int today) {
  // Call parent function
  Habitat::init_day(today);
  grow_daily(today);

  // Increment simulation month on first day of month.
  // On the very first call, `simulation_month` is incremented from -1 to 0.
  if (Date(today, 0).get_day_of_month() == 0 || simulation_month == -1)
    simulation_month++;
  assert(simulation_month >= 0);

  // Get the vector address for the current air temperature.
  assert(!settings.air_temp_monthly.empty());
  const int air_temp_id =
      simulation_month % settings.air_temp_monthly.size();
  assert(air_temp_id >= 0 &&
         air_temp_id < settings.air_temp_monthly.size());

  air_temperature = settings.air_temp_monthly[air_temp_id];
}

void SimpleHabitat::remove_eaten_forage(const ForageMass& eaten_forage) {
  // call parent class implementation
  Habitat::remove_eaten_forage(eaten_forage);

  // create temporary grass object. Remove eaten mass from it, and
  // then assign it to the actual grass object.
  GrassForage new_grass = grass.get_forage();
  if (new_grass.get_mass() - eaten_forage[ForageType::Grass] < 0.0)
    throw std::logic_error(
        "FaunaSim::SimpleHabitat::remove_eaten_forage() "
        "Eaten grass exceeds available grass.\n"
        "Available: " +
        std::to_string(grass.get_forage().get_mass()) +
        " kg/km²\n"
        "Eaten: " +
        std::to_string(eaten_forage[ForageType::Grass]) + " kg/km²");
  new_grass.set_mass(new_grass.get_mass() - eaten_forage[ForageType::Grass]);
  grass.set_forage(new_grass);
}
