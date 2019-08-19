#include "catch.hpp"
#include "habitat.h"
#include "dummy_habitat.h"
#include "habitat_data.h"
using namespace Fauna;
using namespace Fauna::Output;

TEST_CASE("Fauna::Habitat", "") {
  // Since Habitat is an abstract class, we use the simple
  // class DummyHabitat for testing the base class functionality.

  DummyHabitat habitat;

  SECTION("kill()") {
    CHECK(! habitat.is_dead());
    habitat.kill();
    CHECK(habitat.is_dead());
  }

  SECTION("init_day()") {
    // init_day()
    CHECK_THROWS(habitat.init_day(-1));
    CHECK_THROWS(habitat.init_day(365));
    const int DAY = 34;
    habitat.init_day(DAY);
    CHECK(habitat.get_day_public() == DAY);

    habitat.kill();
    CHECK_THROWS(habitat.init_day(DAY));
  }

  SECTION("output") {
    // initialized with zero output.
    const int COUNT = 22;            // day count (even number!)
    const ForageMass EATEN_AVG(54);  // eaten total per day

    habitat.init_day(23);

    // remove twice in the same day, but keeping sum to given value
    habitat.remove_eaten_forage(EATEN_AVG * 0.4);
    habitat.remove_eaten_forage(EATEN_AVG * 0.6);

    SECTION("check this day") {
      const HabitatData out = ((const Habitat&)habitat).get_todays_output();
      // Check eaten forage per day as a sample.
      CHECK(out.eaten_forage[FT_GRASS] == Approx(EATEN_AVG[FT_GRASS]));
    }

    SECTION("init next day") {
      habitat.init_day(24);
      const HabitatData out = ((const Habitat&)habitat).get_todays_output();
      // The values should be reset to zero.
      CHECK(out.eaten_forage[FT_GRASS] == 0.0);
    }
  }
}
