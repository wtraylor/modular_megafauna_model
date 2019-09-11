/**
 * \file
 * \brief Unit test for Fauna::FeedHerbivores.
 * \copyright ...
 * \date 2019
 */
#include <list>
#include "catch.hpp"
#include "dummy_herbivore.h"
#include "dummy_hft.h"
#include "feed_herbivores.h"
#include "forage_distribution_algorithms.h"
#include "habitat_forage.h"
#include "hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::FeedHerbivores") {
  CHECK_THROWS(FeedHerbivores(NULL));

  // create objects
  const HftList HFTS = create_hfts(3, Parameters());
  const double DENS = 1.0;  // irrelevant in this test
  FeedHerbivores feed(new DistributeForageEqually());

  // these variables are set in each test section.
  HabitatForage AVAILABLE;
  HerbivoreVector herbivores;

  SECTION("no herbivores") {
    // set some arbitrary forage
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      AVAILABLE[*ft].set_mass(123.4);

    const ForageMass OLD_AVAIL = AVAILABLE.get_mass();
    feed(AVAILABLE, herbivores);

    // no changes
    CHECK(AVAILABLE.get_mass() == OLD_AVAIL);
  }

  SECTION("single herbivore") {
    DummyHerbivore herbi(&HFTS[0], DENS);

    // create some arbitrary demand (different for each forage type)
    ForageMass DEMAND;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      DEMAND.set(*ft, i++);
    herbi.set_demand(DEMAND);

    herbivores.push_back(&herbi);

    SECTION("more available than demanded") {
      // set available forage slightly higher than demand
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(DEMAND[*ft] * 1.1);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      // perform feeding operations
      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++) {
        CHECK(eaten[*ft] == Approx(DEMAND[*ft]).epsilon(.05));
        CHECK(herbi.get_eaten()[*ft] == Approx(DEMAND[*ft]).epsilon(.05));
      }
    }

    SECTION("less available than demanded") {
      // set available forage slightly lower than demand
      const double FRACTION = 0.9;
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(DEMAND[*ft] * FRACTION);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      // perform feeding operations
      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++) {
        CHECK(eaten[*ft] == Approx(DEMAND[*ft] * FRACTION).epsilon(.05));
        CHECK(herbi.get_eaten()[*ft] ==
              Approx(DEMAND[*ft] * FRACTION).epsilon(.05));
      }
    }

    SECTION("nothing available") {
      REQUIRE(AVAILABLE.get_mass() == 0.0);
      feed(AVAILABLE, herbivores);
      CHECK(AVAILABLE.get_mass() == 0.0);  // nothing changed
      CHECK(herbi.get_eaten() == 0.0);
    }

    SECTION("diet switch") {
      // TODO
    }
  }

  SECTION("many herbivores") {
    ForageMass TOTAL_DEMAND;
    // number of herbivores per HFT
    const int HERBI_COUNT = 10;
    std::list<DummyHerbivore> dummylist;
    // create herbivores
    for (HftList::const_iterator hft = HFTS.begin(); hft != HFTS.end(); hft++) {
      for (int i = 0; i < HERBI_COUNT; i++) {
        dummylist.push_back(DummyHerbivore(&*hft, DENS));
        herbivores.push_back(&dummylist.back());
        // set some arbitrary demand
        const ForageMass IND_DEMAND((double)i);
        dummylist.back().set_demand(IND_DEMAND);
        TOTAL_DEMAND += IND_DEMAND;
      }
    }

    SECTION("more available than demanded") {
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(TOTAL_DEMAND[*ft] * 1.1);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::vector<HerbivoreInterface*>::const_iterator itr =
               herbivores.begin();
           itr != herbivores.end(); itr++) {
        const DummyHerbivore& herbi = *((DummyHerbivore*)*itr);
        // check for each forage type
        for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
             ft != FORAGE_TYPES.end(); ft++) {
          CHECK(herbi.get_eaten()[*ft] ==
                Approx(herbi.get_original_demand()[*ft]).epsilon(.05));
          CHECK(eaten[*ft] == Approx(TOTAL_DEMAND[*ft]).epsilon(.05));
        }
      }
    }

    SECTION("less available than demanded") {
      const double FRACTION = .4;
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(TOTAL_DEMAND[*ft] * FRACTION);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::vector<HerbivoreInterface*>::const_iterator itr =
               herbivores.begin();
           itr != herbivores.end(); itr++) {
        const DummyHerbivore& herbi = *((DummyHerbivore*)*itr);
        // check for each forage type
        for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
             ft != FORAGE_TYPES.end(); ft++) {
          CHECK(
              herbi.get_eaten()[*ft] ==
              Approx(herbi.get_original_demand()[*ft] * FRACTION).epsilon(.05));
          CHECK(eaten[*ft] ==
                Approx(TOTAL_DEMAND[*ft] * FRACTION).epsilon(.05));
        }
      }
    }
  }
}

