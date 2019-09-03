#include "simple_habitat.h"
#include <string>

using namespace Fauna;
using namespace Fauna::Demo;

HabitatEnvironment SimpleHabitat::get_environment() const {
  HabitatEnvironment env;
  env.snow_depth = snow_depth;
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

  // Get the vector address for the current snow depth value.
  assert(!settings.snow_depth_monthly.empty());
  const int snow_depth_id =
      simulation_month % settings.snow_depth_monthly.size();
  assert(snow_depth_id >= 0 &&
         snow_depth_id < settings.snow_depth_monthly.size());

  snow_depth = settings.snow_depth_monthly[snow_depth_id];
}

void SimpleHabitat::remove_eaten_forage(const ForageMass& eaten_forage) {
  // call parent class implementation
  Habitat::remove_eaten_forage(eaten_forage);

  // create temporary grass object. Remove eaten mass from it, and
  // then assign it to the actual grass object.
  GrassForage new_grass = grass.get_forage();
  if (new_grass.get_mass() - eaten_forage[FT_GRASS] < 0.0)
    throw std::logic_error(
        "FaunaSim::SimpleHabitat::remove_eaten_forage() "
        "Eaten grass exceeds available grass.");
  new_grass.set_mass(new_grass.get_mass() - eaten_forage[FT_GRASS]);
  grass.set_forage(new_grass);
}
