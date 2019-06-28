#include <catch2/catch.hpp>
#include "dummy_herbivore.h"
using namespace Fauna;

TEST_CASE("DummyHerbivore") {
  Hft hft1;
  hft1.name = "hft1";
  // INITIALIZATION
  CHECK(DummyHerbivore(&hft1, 1.0).get_ind_per_km2() == 1.0);
  CHECK(DummyHerbivore(&hft1, 0.0).get_ind_per_km2() == 0.0);
  CHECK(DummyHerbivore(&hft1, 1.0, 25.0).get_bodymass() == 25.0);
  CHECK(DummyHerbivore(&hft1, 1.0).get_original_demand() == 0.0);
  DummyHerbivore d(&hft1, 1.0);
  CHECK(&d.get_hft() == &hft1);
  CHECK(d.get_eaten() == 0.0);
  CHECK(d.get_original_demand() == 0.0);

  const ForageMass DEMAND(23.9);
  d.set_demand(DEMAND);
  CHECK(d.get_original_demand() == DEMAND);

  const ForageMass EATEN(12.4);
  d.eat(EATEN, Digestibility(.5));
  CHECK(d.get_eaten() == EATEN);
}


