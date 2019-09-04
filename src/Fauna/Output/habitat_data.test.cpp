/**
 * \file
 * \brief Unit test for Fauna::Output::HabitatData.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "habitat_data.h"
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
