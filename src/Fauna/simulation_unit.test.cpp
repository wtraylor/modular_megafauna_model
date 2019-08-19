#include "catch.hpp"
#include "dummy_habitat.h"
#include "population.h"
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("Fauna::SimulationUnit") {
  Hft HFT;
  CHECK_THROWS(SimulationUnit(NULL, new HftPopulationsMap()));
  CHECK_THROWS(SimulationUnit(new DummyHabitat(), NULL));
}

