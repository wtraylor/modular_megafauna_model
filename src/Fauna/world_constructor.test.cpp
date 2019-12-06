/**
 * \file
 * \brief Unit test for Fauna::WorldConstructor.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_habitat.h"
#include "dummy_hft.h"
#include "parameters.h"
#include "population_interface.h"
#include "population_list.h"
#include "simulation_unit.h"
#include "world_constructor.h"
using namespace Fauna;

TEST_CASE("Fauna::WorldConstructor", "") {
  std::shared_ptr<Parameters> params(new Parameters);
  REQUIRE(params->is_valid());

  // prepare HFT list
  HftList hftlist = create_hfts(3, *params);

  WorldConstructor world_cons(params, hftlist);

  SECTION("create_populations() for several HFTs") {
    PopulationList* pops = world_cons.create_populations();
    REQUIRE(pops != NULL);
    for (auto& hft : hftlist) CHECK(pops->exists(*hft));
  }

  SECTION("create_populations() for one HFT") {
    const auto hft = hftlist[0];
    PopulationList* pops = world_cons.create_populations(hft);
    REQUIRE(pops != NULL);
    CHECK(pops->exists(*hft));
    CHECK(pops->get(*hft).get_hft() == *hft);
  }
}
