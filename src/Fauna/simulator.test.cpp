#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "simulator.h"
using namespace Fauna;

TEST_CASE("Fauna::Simulator", "") {
  Fauna::Parameters params;
  REQUIRE(params.is_valid());

  // prepare HFT list
  HftList hftlist = create_hfts(3, params);

  Simulator sim(params);

  SECTION("create_populations() for several HFTs") {
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(hftlist);
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
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(phft);
    REQUIRE(pops.get() != NULL);
    CHECK(pops->size() == 1);
    CHECK(&(*(pops->begin()))->get_hft() == phft);
  }

  SECTION("simulate_day()") {
    // Check simulate_day()
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(hftlist);

    SimulationUnit simunit(
        std::auto_ptr<Habitat>(new DummyHabitat()),
        std::auto_ptr<HftPopulationsMap>(new HftPopulationsMap));
    CHECK_THROWS(sim.simulate_day(-1, simunit, true));
    CHECK_THROWS(sim.simulate_day(366, simunit, true));
    const bool do_herbivores = true;
    for (int d = 0; d < 365; d++) {
      sim.simulate_day(d, simunit, do_herbivores);

      // Check if day has been set correctly.
      CHECK(((DummyHabitat&)simunit.get_habitat()).get_day_public() == d);

      // Note: Various other things could be tested here.
      // But they are becoming more difficult to formulate as unit tests
      // because Simulator::simulate_day() and SimulateDay are on a high
      // level in the program hierarchy.
    }
  }
}
