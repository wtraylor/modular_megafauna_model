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

}  // anonymous namespace

// TEST CASES IN ALPHABETICAL ORDER, PLEASE

TEST_CASE("Dummies", "") {
  SECTION("DummyPopulation") {
    DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
    DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);
    DummyPopulation pop = DummyPopulation(&hft1);
    pop.create_offspring(1.0);
    REQUIRE(pop.get_list().size() == 1);
  }
}
