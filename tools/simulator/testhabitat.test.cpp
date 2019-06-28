#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "testhabitat.h"
using namespace Fauna;

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

TEST_CASE("FaunaSim::SimpleHabitat", "") {
  SimpleHabitat::Parameters settings;
  settings.grass.init_mass = 1.0;
  settings.grass.saturation = 3.0;

  // create a habitat with some populations
  const Fauna::Parameters params;
  Simulator sim(params);
  SimpleHabitat habitat(settings);

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

TEST_CASE("FaunaSim::HabitatGroup", "") {
  // Make sure the group creates its habitats
  HabitatGroup group(1.0, 1.0);  // lon,lat
  group.reserve(5);
  for (int i = 1; i < 5; i++) {
    // create a simulation unit
    std::auto_ptr<Habitat> habitat(new DummyHabitat());
    std::auto_ptr<HftPopulationsMap> populations(new HftPopulationsMap());
    std::auto_ptr<SimulationUnit> simunit(
        new SimulationUnit(habitat, populations));
    // add it to the group
    group.add(simunit);
    // Check if it has been added properly
    CHECK(group.size() == i);
    CHECK(group.get_vector().size() == i);
  }
  // Make sure the references are pointing correctly to the objects
  const std::vector<SimulationUnit*> refs = group.get_vector();
  HabitatGroup::const_iterator itr = group.begin();
  int j = 0;
  while (itr != group.end()) {
    CHECK(refs[j] == *itr);
    j++;
    itr++;
  }
}

TEST_CASE("FaunaSim::HabitatGroupList", "") {
  // Make sure the group creates its habitats
  HabitatGroupList gl;
  gl.reserve(5);

  // add some habitat groups
  for (int i = 1; i < 5; i++) {
    HabitatGroup& group =
        gl.add(std::auto_ptr<HabitatGroup>(new HabitatGroup(i, i)));
    for (int j = 1; j < 4; j++) {
      // create a simulation unit
      std::auto_ptr<Habitat> habitat(new DummyHabitat());
      std::auto_ptr<HftPopulationsMap> populations(new HftPopulationsMap());
      std::auto_ptr<SimulationUnit> simunit(
          new SimulationUnit(habitat, populations));
      // add it to the group
      group.add(simunit);
    }
    CHECK(gl.size() == i);
  }
  // Don’t allow adding a group with same coordinates twice
  CHECK_THROWS(gl.add(std::auto_ptr<HabitatGroup>(new HabitatGroup(1, 1))));
}
