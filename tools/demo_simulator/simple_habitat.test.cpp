#include "catch.hpp"
#include "population_list.h"
#include "simple_habitat.h"

using namespace Fauna;
using namespace Fauna::Demo;

TEST_CASE("Fauna::Demo::SimpleHabitat", "") {
  SimpleHabitat::Parameters settings;
  settings.grass.init_mass = 1.0;
  settings.grass.saturation = 3.0;

  // create a habitat with some populations
  const Fauna::Parameters params;
  static const std::string AGG_UNIT = "aggregation_unit";
  SimpleHabitat habitat(settings, AGG_UNIT);

  CHECK(habitat.get_aggregation_unit() == AGG_UNIT);

  SECTION("Initialization") {
    CHECK(habitat.get_available_forage().grass.get_fpc() ==
          Approx(settings.grass.fpc));
    CHECK(habitat.get_available_forage().grass.get_mass() ==
          Approx(settings.grass.init_mass));
    CHECK(habitat.get_available_forage().grass.get_digestibility() ==
          Approx(settings.grass.digestibility[0]));
  }

  SECTION("Remove forage") {
    const HabitatForage avail = habitat.get_available_forage();

    SECTION("Remove some forage") {
      const ForageMass eaten = avail.get_mass() * 0.5;
      habitat.remove_eaten_forage(eaten);
      // check each forage type with Approx()
      for (ForageMass::const_iterator i = eaten.begin(); i != eaten.end(); i++)
        CHECK(habitat.get_available_forage().get_mass()[i->first] ==
              Approx(avail.get_mass()[i->first] - i->second));
    }

    SECTION("Remove all forage") {
      const ForageMass eaten = avail.get_mass();
      habitat.remove_eaten_forage(eaten);
      for (ForageMass::const_iterator i = eaten.begin(); i != eaten.end(); i++)
        CHECK(habitat.get_available_forage().get_mass()[i->first] ==
              Approx(0.0));
    }

    SECTION("Remove more forage than is available") {
      const ForageMass too_much = avail.get_mass() * 1.1;
      CHECK_THROWS(habitat.remove_eaten_forage(too_much));
    }
  }
}
