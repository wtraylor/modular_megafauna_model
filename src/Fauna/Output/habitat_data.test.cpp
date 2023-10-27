// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Output::HabitatData.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "habitat_data.h"

#include "catch.hpp"
using namespace Fauna;
using namespace Fauna::Output;

TEST_CASE("Fauna::Output::HabitatData", "") {
  SECTION("Exceptions") {
    HabitatData d1, d2;
    CHECK_THROWS(d1.merge(d2, 0, 0));
    CHECK_THROWS(d1.merge(d2, -1, 1));
    CHECK_THROWS(d1.merge(d2, 1, -1));
  }

  // The values of the merge are not checked here because
  // they are given by Fauna::ForageValues<>::merge()
  // and Fauna::average().
}
