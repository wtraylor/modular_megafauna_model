//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Unit tests for megafauna herbivores.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
//////////////////////////////////////////////////////////////////////////

#include <algorithm> // for std::max()
#include <cmath>   // for exp() and pow()
#include <memory>  // for std::auto_ptr
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "digestibility.h"
#include "energetics.h"
#include "environment.h"
#include "forageclasses.h"
#include "forageenergy.h"
#include "foraging.h"
#include "framework.h"
#include "herbivore.h"
#include "hft.h"
#include "mortality.h"
#include "parameters.h"
#include "population.h"
#include "reproduction.h"
#include "simulation_unit.h"
#include "testhabitat.h"
#include "utils.h"
using namespace Fauna;

using namespace Fauna;
using namespace FaunaOut;
using namespace FaunaSim;

namespace {
struct DistributeForageDummy : public DistributeForage {
  virtual void operator()(const HabitatForage& available,
                          ForageDistribution& forage_distribution) const {
    // TODO
  }
};

}  // anonymous namespace

// TEST CASES IN ALPHABETICAL ORDER, PLEASE

TEST_CASE("Dummies", "") {
  Hft hft1;
  hft1.name = "hft1";
  SECTION("DummyHerbivore") {
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

  SECTION("DummyPopulation") {
    DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
    DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);
    DummyPopulation pop = DummyPopulation(&hft1);
    pop.create_offspring(1.0);
    REQUIRE(pop.get_list().size() == 1);
  }
}
