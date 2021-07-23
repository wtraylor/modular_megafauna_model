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

  SECTION("Unit test constructor") {
    CHECK_THROWS(World(NULL, NULL));
    const std::shared_ptr<const Parameters> params(new Parameters);
    CHECK_THROWS(World(params, NULL));
    static const HftList hftlist = create_hfts(3, *params);
    static const auto hftlist_ptr = std::shared_ptr<const HftList>(&hftlist);
    CHECK_THROWS(World(NULL, hftlist_ptr));
    CHECK_NOTHROW(World(params, hftlist_ptr));
    REQUIRE(World(params, hftlist_ptr).is_activated());
    REQUIRE(World(params, hftlist_ptr).get_sim_units().empty());
  }

  SECTION("simulate_day()") {
    const std::shared_ptr<const Parameters> params(new Parameters);
    static const HftList hftlist = create_hfts(2, *params);
    static const auto hftlist_ptr = std::shared_ptr<const HftList>(&hftlist);
    World world(params, hftlist_ptr);
    world.simulate_day(Date(0, 0), true);
    // Run for some days
    CHECK_NOTHROW(world.simulate_day(Date(0, 1), true));
    CHECK_NOTHROW(world.simulate_day(Date(0, 2), true));
    CHECK_NOTHROW(world.simulate_day(Date(0, 3), true));
    // Try to simulate the same day again.
    CHECK_THROWS(world.simulate_day(Date(0, 3), true));
    // Try to simulate the previous day again
    CHECK_THROWS(world.simulate_day(Date(0, 2), true));
    // Try to simulate some bad arbitrary days.
    CHECK_THROWS(world.simulate_day(Date(1, 4), true));
    CHECK_THROWS(world.simulate_day(Date(0, 14), true));
    CHECK_THROWS(world.simulate_day(Date(3, 14), true));
    CHECK_THROWS(world.simulate_day(Date(3, 10), true));
    CHECK_THROWS(world.simulate_day(Date(3, 10), true));

    // Check that the year boundary works.
    for (int day = 4; day < 365; day++)
      CHECK_NOTHROW(world.simulate_day(Date(0, day), true));
    CHECK_NOTHROW(world.simulate_day(Date(1, 0), true));
    CHECK_NOTHROW(world.simulate_day(Date(1, 1), true));
    CHECK_NOTHROW(world.simulate_day(Date(1, 2), true));
  }
}
