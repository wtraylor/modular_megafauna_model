#include "catch.hpp"
#include "logistic_grass.h"

using namespace Fauna;
using namespace Fauna::Demo;

TEST_CASE("FaunaSim::LogisticGrass", "") {
  LogisticGrass::Parameters grass_settings;
  grass_settings.reserve = 2.0;  // just an arbitrary positive number
  grass_settings.init_mass = 1.0;
  grass_settings.saturation = 10 * grass_settings.init_mass;

  const int day = 1;  // day of the year

  SECTION("Grass initialization") {
    LogisticGrass grass(grass_settings);
    CHECK(grass.get_forage().get_mass() == Approx(grass_settings.init_mass));
    CHECK(grass.get_forage().get_digestibility() ==
          Approx(grass_settings.digestibility[0]));
    CHECK(grass.get_forage().get_fpc() == Approx(grass_settings.fpc));

    // exceptions
    CHECK_THROWS(grass.grow_daily(-1));
    CHECK_THROWS(grass.grow_daily(365));
  }

  // Let the grass grow for one day and compare before and after
  // Now we fill in new growth and decay values, so we need to remove the
  // default first.
  grass_settings.growth_monthly.clear();
  grass_settings.decay_monthly.clear();

  SECTION("No grass growth") {
    grass_settings.growth_monthly.push_back(0.0);
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() == Approx(before.get_mass()));
  }

  SECTION("Positive grass growth") {
    grass_settings.growth_monthly.push_back(0.1);
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    // Let the grass grow for one day and check it’s greater
    INFO("grass_settings.growth_monthly == "
         << grass_settings.growth_monthly[0]);
    INFO("grass_settings.decay_monthly == " << grass_settings.decay_monthly[0]);

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() > before.get_mass());

    // Let it grow for very long and check that it reaches saturation
    for (int i = 0; i < 1000000; i++) grass.grow_daily(i % 365);
    CHECK(grass.get_forage().get_mass() == Approx(grass_settings.saturation));
  }

  SECTION("Negative grass growth") {
    // decay is greater than growth
    grass_settings.growth_monthly.push_back(0.1);
    grass_settings.decay_monthly.push_back(0.2);

    LogisticGrass grass(grass_settings);

    // Let the grass grow for one day and check it’s greater

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() < before.get_mass());
  }

  SECTION("Multi-months growth") {
    const double GROWTH = 0.001;

    // Growth in first month
    grass_settings.growth_monthly.push_back(GROWTH);
    // Growth in second month
    grass_settings.growth_monthly.push_back(0.0);

    // Decay is constant zero. It’s not tested explicitly, but we assume
    // that it works like growth.
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    int d = 0;
    // Let it grow for January (31 days)
    for (; d <= 31 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      if (after.get_mass() < grass_settings.saturation)
        CHECK(after.get_mass() > before.get_mass());
    }

    // Let it grow for February (28 days)
    for (; d <= 31 + 28 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      CHECK(after.get_mass() == Approx(before.get_mass()));
    }

    // Let it grow for March (31 days), now recycling the first value
    for (; d <= 31 + 28 + 31 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      if (after.get_mass() < grass_settings.saturation)
        CHECK(after.get_mass() > before.get_mass());
    }
  }
}

