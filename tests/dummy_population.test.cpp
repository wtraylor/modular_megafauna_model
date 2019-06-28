#include <catch2/catch.hpp>
#include "dummy_population.h"
using namespace Fauna;

TEST_CASE("DummyPopulation") {
  Hft hft1;
  hft1.name = "hft1";
  DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
  DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);
  DummyPopulation pop = DummyPopulation(&hft1);
  pop.create_offspring(1.0);
  REQUIRE(pop.get_list().size() == 1);
}
