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
#include "date.h"
#include "dummy_hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("FAUNA::World", "") {
  SECTION("Dummy Constructor") {
    World w;
    REQUIRE(!w.is_activated());
    CHECK_THROWS(w.get_params());
    CHECK_THROWS(w.create_simulation_unit(NULL));
    CHECK_NOTHROW(w.simulate_day(Date(1, 2), true));
    CHECK_NOTHROW(w.simulate_day(Date(1, 2), false));
  }

  static const std::shared_ptr<const Parameters> PARAMS(new Parameters);
  static const std::shared_ptr<const HftList> HFTLIST(create_hfts(3, *PARAMS));
  SECTION("Unit test constructor") {
    CHECK_THROWS(World(NULL, NULL));
    CHECK_THROWS(World(PARAMS, NULL));
    CHECK_THROWS(World(NULL, HFTLIST));
    CHECK_NOTHROW(World(PARAMS, HFTLIST));
    REQUIRE(World(PARAMS, HFTLIST).is_activated());
    REQUIRE(World(PARAMS, HFTLIST).get_sim_units().empty());
  }

  SECTION("simulate_day()") {
    World world(PARAMS, HFTLIST);
    world.simulate_day(Date(0, 0), true);
    // Run for some days
    CHECK_NOTHROW(world.simulate_day(Date(1, 0), true));
    CHECK_NOTHROW(world.simulate_day(Date(2, 0), true));
    CHECK_NOTHROW(world.simulate_day(Date(3, 0), true));
    // Try to simulate the same day again.
    CHECK_THROWS(world.simulate_day(Date(3, 0), true));
    // Try to simulate the previous day again
    CHECK_THROWS(world.simulate_day(Date(2, 0), true));
    // Try to simulate some bad arbitrary days.
    CHECK_THROWS(world.simulate_day(Date(4, 1), true));
    CHECK_THROWS(world.simulate_day(Date(14, 0), true));
    CHECK_THROWS(world.simulate_day(Date(14, 3), true));
    CHECK_THROWS(world.simulate_day(Date(10, 3), true));
    CHECK_THROWS(world.simulate_day(Date(10, 3), true));

    // Check that the year boundary works.
    for (int day = 4; day < 365; day++)
      CHECK_NOTHROW(world.simulate_day(Date(day, 0), true));
    CHECK_NOTHROW(world.simulate_day(Date(0, 1), true));
    CHECK_NOTHROW(world.simulate_day(Date(1, 1), true));
    CHECK_NOTHROW(world.simulate_day(Date(2, 1), true));
  }
}
