#include "catch.hpp"
#include "dummy_habitat.h"
#include "population_list.h"
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("Fauna::SimulationUnit") {
  Hft HFT;
  CHECK_THROWS(SimulationUnit(NULL, new PopulationList()));
  CHECK_THROWS(SimulationUnit(new DummyHabitat(), NULL));
}

