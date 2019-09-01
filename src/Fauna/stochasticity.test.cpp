#include "catch.hpp"
#include "stochasticity.h"

using namespace Fauna;

TEST_CASE("Fauna::get_random_fraction", "") {
  for (int i = 0; i < 100; i++) {
    const double r = get_random_fraction(i);
    CHECK(r <= 1.0);
    CHECK(r >= 0.0);
  }
}

