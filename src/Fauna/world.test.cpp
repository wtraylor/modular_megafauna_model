// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::World.
 * \copyright LGPL-3.0-or-later
 * \date 2020
 */
#include "world.h"
#include "catch.hpp"
#include "cohort_population.h"
#include "date.h"
#include "dummy_habitat.h"
#include "dummy_hft.h"
#include "parameters.h"
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("FAUNA::World", "") {
  static const std::shared_ptr<const Parameters> PARAMS(new Parameters);
  static const std::shared_ptr<const HftList> HFTLIST(create_hfts(3, *PARAMS));

  CHECK_THROWS(World(NULL, NULL));
  CHECK_THROWS(World(NULL, HFTLIST));
  CHECK_THROWS(World(PARAMS, NULL));
  REQUIRE_NOTHROW(World(PARAMS, HFTLIST));
  CHECK_THROWS(World(PARAMS, HFTLIST).create_simulation_unit(NULL));
  CHECK_NOTHROW(World(PARAMS, HFTLIST).get_params());
  CHECK_NOTHROW(World(PARAMS, HFTLIST).simulate_day(Date(1, 2)));
  CHECK_NOTHROW(World(PARAMS, HFTLIST).simulate_day(Date(1, 2)));

  // Fauna::World should also work without HFTs: for simulations without any
  // herbivores, but with producing MMM output files.
  CHECK_NOTHROW(World(PARAMS, std::shared_ptr<const HftList>(new HftList)));

  SECTION("Dummy Constructor (deprecated)") {
    World w;
    REQUIRE(!w.is_activated());
    CHECK_THROWS(w.get_params());
    CHECK_THROWS(w.create_simulation_unit(NULL));
    CHECK_NOTHROW(w.simulate_day(Date(1, 2)));
    CHECK_NOTHROW(w.simulate_day(Date(1, 2)));
  }

  SECTION("Unit test constructor") {
    CHECK_THROWS(World(NULL, NULL));
    CHECK_THROWS(World(PARAMS, NULL));
    CHECK_THROWS(World(NULL, HFTLIST));
    CHECK_NOTHROW(World(PARAMS, HFTLIST));
    REQUIRE(World(PARAMS, HFTLIST).get_sim_units().empty());
  }

  SECTION("simulate_day()") {
    World world(PARAMS, HFTLIST);
    world.simulate_day(Date(0, 0));
    // Run for some days
    CHECK_NOTHROW(world.simulate_day(Date(1, 0)));
    CHECK_NOTHROW(world.simulate_day(Date(2, 0)));
    CHECK_NOTHROW(world.simulate_day(Date(3, 0)));
    // Try to simulate the same day again.
    CHECK_THROWS(world.simulate_day(Date(3, 0)));
    // Try to simulate the previous day again
    CHECK_THROWS(world.simulate_day(Date(2, 0)));
    // Try to simulate some bad arbitrary days.
    CHECK_THROWS(world.simulate_day(Date(4, 1)));
    CHECK_THROWS(world.simulate_day(Date(14, 0)));
    CHECK_THROWS(world.simulate_day(Date(14, 3)));
    CHECK_THROWS(world.simulate_day(Date(10, 3)));
    CHECK_THROWS(world.simulate_day(Date(10, 3)));

    // Check that the year boundary works.
    for (int day = 4; day < 365; day++)
      CHECK_NOTHROW(world.simulate_day(Date(day, 0)));
    CHECK_NOTHROW(world.simulate_day(Date(0, 1)));
    CHECK_NOTHROW(world.simulate_day(Date(1, 1)));
    CHECK_NOTHROW(world.simulate_day(Date(2, 1)));

    SECTION("no herbivores") {
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      World::SimDayOptions opts;
      opts.do_herbivores = false;
      CHECK_NOTHROW(world.simulate_day(Date(3, 1), opts));
      CHECK_NOTHROW(world.simulate_day(Date(4, 1), opts));
      CHECK_NOTHROW(world.simulate_day(Date(5, 1), opts));
    }

    SECTION("reset date") {
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      World::SimDayOptions opts;
      opts.reset_date = true;
      CHECK_NOTHROW(world.simulate_day(Date(0, 0), opts));
      CHECK_NOTHROW(world.simulate_day(Date(3, 2), opts));
      opts.reset_date = false;
      CHECK_NOTHROW(world.simulate_day(Date(4, 2), opts));
      CHECK_NOTHROW(world.simulate_day(Date(5, 2), opts));
      CHECK_THROWS(world.simulate_day(Date(0, 0), opts));
    }

    SECTION("deprecated simulate_day()") {
      CHECK_NOTHROW(world.simulate_day(Date(3, 1), true));
      CHECK_NOTHROW(world.simulate_day(Date(4, 1), false));
    }
  }

  SECTION("Unequal habitat count per aggregation unit") {
    World world(PARAMS, HFTLIST);
    SECTION("Good habitat count: 4") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      CHECK_NOTHROW(world.simulate_day(Date(1, 0)));
    }
    SECTION("Good habitat count: 2-2-2") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      CHECK_NOTHROW(world.simulate_day(Date(1, 0)));
    }
    SECTION("Good habitat count: 1-1") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      CHECK_NOTHROW(world.simulate_day(Date(1, 0)));
    }
    SECTION("Bad habitat count: 2-2-1") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      CHECK_THROWS(world.simulate_day(Date(1, 0)));
    }
    SECTION("Bad habitat count: 2-3") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      CHECK_THROWS(world.simulate_day(Date(1, 0)));
    }
    SECTION("Bad habitat count: 1-3") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      CHECK_THROWS(world.simulate_day(Date(1, 0)));
    }
    SECTION("Bad habitat count: 4-2-3") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
      CHECK_THROWS(world.simulate_day(Date(1, 0)));
    }
  }

  SECTION("Every HFT in every habitat") {
    REQUIRE(PARAMS->herbivore_type == HerbivoreType::Cohort);
    REQUIRE(PARAMS->one_hft_per_habitat == false);
    World world(PARAMS, HFTLIST);
    REQUIRE(HFTLIST->size() > 1);

    SECTION("1 habitat, 1 aggregation unit") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check all HFTs are in each habitat.
      for (const auto& sim_unit : world.get_sim_units()) {
        REQUIRE(sim_unit.get_populations().size() == HFTLIST->size());
        for (const auto& hft : *HFTLIST) {
          bool hft_found = false;
          for (const auto& pop : sim_unit.get_populations())
            hft_found |= (((CohortPopulation*)pop.get())->get_hft() == *hft);
          CHECK(hft_found);
        }
      }
    }

    SECTION("2 habitats, 1 aggregation unit") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check all HFTs are in each habitat.
      for (const auto& sim_unit : world.get_sim_units()) {
        REQUIRE(sim_unit.get_populations().size() == HFTLIST->size());
        for (const auto& hft : *HFTLIST) {
          bool hft_found = false;
          for (const auto& pop : sim_unit.get_populations())
            hft_found |= (((CohortPopulation*)pop.get())->get_hft() == *hft);
          CHECK(hft_found);
        }
      }
    }

    SECTION("1 habitats, 2 aggregation unit") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check all HFTs are in each habitat.
      for (const auto& sim_unit : world.get_sim_units()) {
        REQUIRE(sim_unit.get_populations().size() == HFTLIST->size());
        for (const auto& hft : *HFTLIST) {
          bool hft_found = false;
          for (const auto& pop : sim_unit.get_populations())
            hft_found |= (((CohortPopulation*)pop.get())->get_hft() == *hft);
          CHECK(hft_found);
        }
      }
    }

    SECTION("2 habitats, 3 aggregation unit") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("3")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check all HFTs are in each habitat.
      for (const auto& sim_unit : world.get_sim_units()) {
        REQUIRE(sim_unit.get_populations().size() == HFTLIST->size());
        for (const auto& hft : *HFTLIST) {
          bool hft_found = false;
          for (const auto& pop : sim_unit.get_populations())
            hft_found |= (((CohortPopulation*)pop.get())->get_hft() == *hft);
          CHECK(hft_found);
        }
      }
    }
  }

  SECTION("Habitat count as multiple of HFT count") {
    std::shared_ptr<Parameters> params(new Parameters);
    params->one_hft_per_habitat = true;
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    World world(params, HFTLIST);

    REQUIRE(HFTLIST->size() > 1);

    SECTION("Habitat count == 0") {
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
    }

    SECTION("Habitat count == HFT count") {
      // Create 3 aggregation units with one habitat for each HFT.
      for (int i = 0; i < HFTLIST->size(); i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
      }
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check that all HFT populations are created.
      for (const auto& hft : *HFTLIST) {
        int hft_ctr = 0;
        for (const auto& sim_unit : world.get_sim_units()) {
          REQUIRE(sim_unit.get_populations().size() == 1);
          const CohortPopulation* pop =
              (CohortPopulation*)sim_unit.get_populations().front().get();
          if (pop->get_hft() == *hft) hft_ctr++;
        }
        CHECK(hft_ctr == 3);  // This HFT once in each of the 3 agg. units
      }
    }

    SECTION("Habitat count == 2 * HFT count") {
      // Create 3 aggregation units with 2 habitats for each HFT.
      for (int i = 0; i < 2 * HFTLIST->size(); i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
      }
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check that all HFT populations are created.
      for (const auto& hft : *HFTLIST) {
        int hft_ctr = 0;
        for (const auto& sim_unit : world.get_sim_units()) {
          REQUIRE(sim_unit.get_populations().size() == 1);
          const CohortPopulation* pop =
              (CohortPopulation*)sim_unit.get_populations().front().get();
          if (pop->get_hft() == *hft) hft_ctr++;
        }
        CHECK(hft_ctr == 2 * 3);  // HFT 2 times in each of the 3 agg. units
      }
    }

    SECTION("Habitat count == 3 * HFT count") {
      // Create 4 aggregation units with 3 habitats for each HFT.
      for (int i = 0; i < 3 * HFTLIST->size(); i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("4")));
      }
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
      // Check that all HFT populations are created.
      for (const auto& hft : *HFTLIST) {
        int hft_ctr = 0;
        for (const auto& sim_unit : world.get_sim_units()) {
          REQUIRE(sim_unit.get_populations().size() == 1);
          const CohortPopulation* pop =
              (CohortPopulation*)sim_unit.get_populations().front().get();
          if (pop->get_hft() == *hft) hft_ctr++;
        }
        CHECK(hft_ctr == 3 * 4);  // HFT 3 times in each of the 4 agg. units
      }
    }

    SECTION("Habitat count == 1") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
    }

    SECTION("Habitat count == HFT count - 1") {
      for (int i = 0; i < HFTLIST->size() - 1; i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
      }
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
    }

    SECTION("Habitat count == HFT count + 1") {
      for (int i = 0; i < HFTLIST->size() + 1; i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
      }
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
    }

    SECTION("Habitat count == 2 * HFT count - 1") {
      for (int i = 0; i < 2 * HFTLIST->size() - 1; i++) {
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("1")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("2")));
        world.create_simulation_unit(
            std::shared_ptr<Habitat>(new DummyHabitat("3")));
      }
      CHECK_THROWS(world.simulate_day(Date(0, 0)));
    }
  }

  SECTION("one_hft_per_habitat doesn’t matter without HFTs") {
    std::shared_ptr<Parameters> params(new Parameters);
    params->one_hft_per_habitat = true;
    World world(PARAMS, std::shared_ptr<const HftList>(new HftList));

    SECTION("1 habitat") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
    }
    SECTION("2 habitat") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
    }
    SECTION("2 habitat in 2 agg. units") {
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("1")));
      world.create_simulation_unit(
          std::shared_ptr<Habitat>(new DummyHabitat("2")));
      CHECK_NOTHROW(world.simulate_day(Date(0, 0)));
    }
  }
}
