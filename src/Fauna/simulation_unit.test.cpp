#include <catch2/catch.hpp>
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("Fauna::SimulationUnit") {
  Hft HFT;
  CHECK_THROWS(SimulationUnit(std::auto_ptr<Habitat>(NULL),
                              std::auto_ptr<HftPopulationsMap>()));
  CHECK_THROWS(SimulationUnit(std::auto_ptr<Habitat>(new DummyHabitat()),
                              std::auto_ptr<HftPopulationsMap>(NULL)));
}

