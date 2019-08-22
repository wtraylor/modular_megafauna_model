#include "catch.hpp"
#include "dummy_habitat.h"
#include "dummy_hft.h"
#include "parameters.h"
#include "population.h"
#include "simulation_unit.h"
#include "world_constructor.h"
using namespace Fauna;

TEST_CASE("Fauna::WorldConstructor", "") {
  Fauna::Parameters params;
  REQUIRE(params.is_valid());

  // prepare HFT list
  HftList hftlist = create_hfts(3, params);

  WorldConstructor world_cons(params, hftlist);

  SECTION("create_populations() for several HFTs") {
    std::auto_ptr<HftPopulationsMap> pops = world_cons.create_populations();
    REQUIRE(pops.get() != NULL);
    CHECK(pops->size() == hftlist.size());
    // find all HFTs
    HftList::const_iterator itr_hft = hftlist.begin();
    while (itr_hft != hftlist.end()) {
      bool found_hft = false;
      HftPopulationsMap::const_iterator itr_pop = pops->begin();
      while (itr_pop != pops->end()) {
        if ((*itr_pop)->get_hft() == *itr_hft) found_hft = true;
        itr_pop++;
      }
      CHECK(found_hft);
      itr_hft++;
    }
  }

  SECTION("create_populations() for one HFT") {
    const Hft* phft = &hftlist[0];
    std::auto_ptr<HftPopulationsMap> pops = world_cons.create_populations(phft);
    REQUIRE(pops.get() != NULL);
    CHECK(pops->size() == 1);
    CHECK(&(*(pops->begin()))->get_hft() == phft);
  }
}

