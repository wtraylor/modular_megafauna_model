// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::World.
 * \copyright LGPL-3.0-or-later
 * \date 2020
 */
#include "catch.hpp"
#include "date.h"
#include "world.h"
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
}
