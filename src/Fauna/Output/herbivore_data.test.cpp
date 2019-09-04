/**
 * \file
 * \brief Unit test for Fauna::HerbivoreData.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_hft.h"
#include "herbivore_data.h"
using namespace Fauna;
using namespace Fauna::Output;

TEST_CASE("FaunaOut::HerbivoreData", "") {
  SECTION("Exceptions") {
    HerbivoreData d1, d2;
    CHECK_THROWS(d1.merge(d2, 0, 0));
    CHECK_THROWS(d1.merge(d2, -1, 1));
    CHECK_THROWS(d1.merge(d2, 1, -1));
    CHECK_THROWS(HerbivoreData::create_datapoint(std::vector<HerbivoreData>()));
  }

  SECTION("create_datapoint()") {
    HerbivoreData d0, d1, d2, d3;
    REQUIRE(d0.inddens == 0.0);  // zero initialization

    // prepare some arbitrary data
    d1.inddens = 1;
    d2.inddens = 2;
    d3.inddens = 3;
    d1.expenditure = 1;
    d2.expenditure = 2;
    d3.expenditure = 3;
    d1.mortality[MF_BACKGROUND] = .1;
    d2.mortality[MF_BACKGROUND] = .2;
    d3.mortality[MF_BACKGROUND] = .3;
    d1.mortality[MF_LIFESPAN] = .5;

    // put them in a vector
    std::vector<HerbivoreData> vec;
    vec.push_back(d1);
    vec.push_back(d2);
    vec.push_back(d3);

    // create a datapoint
    HerbivoreData datapoint = HerbivoreData::create_datapoint(vec);

    // check if values are okay
    CHECK(datapoint.inddens == Approx(6.0));  // sum of all items
    // averages weighted by inddens:
    CHECK(datapoint.expenditure == Approx((1.0 + 2.0 * 2.0 + 3.0 * 3.0) / 6.0));
    CHECK(datapoint.mortality[MF_BACKGROUND] ==
          Approx((.1 + .2 * 2.0 + .3 * 3.0) / 6.0));
    // this mortality factor was only present in one item:
    CHECK(datapoint.mortality[MF_LIFESPAN] == Approx(.5));
  }

  SECTION("merge()") {
    HerbivoreData d1, d2;
    d1.inddens = 1;
    d2.inddens = 2;
    d1.expenditure = 1.0;
    d2.expenditure = 2.0;
    d1.mortality[MF_BACKGROUND] = .1;
    d2.mortality[MF_BACKGROUND] = .2;
    d1.mortality[MF_LIFESPAN] = .5;
    // no lifespan mortality in d2

    SECTION("equal weights") {
      d1.merge(d2, 1.0, 1.0);
      // simple average:
      CHECK(d1.inddens == Approx(1.5));
      CHECK(d1.mortality[MF_BACKGROUND] == Approx(.15));
      CHECK(d1.mortality[MF_LIFESPAN] == 0.0);  // was only in one datapoint
      // average weighted by inddens:
      CHECK(d1.expenditure == Approx((1.0 + 2.0 * 2.0) / 3.0));
    }

    SECTION("different weights") {
      d1.merge(d2, 1.0, 2.0);
      // simple average:
      CHECK(d1.inddens == Approx((1.0 + 2.0 * 2.0) / (1.0 + 2.0)));
      CHECK(d1.mortality[MF_BACKGROUND] ==
            Approx((.1 + 2.0 * .2) / (1.0 + 2.0)));
      CHECK(d1.mortality[MF_LIFESPAN] == 0.0);  // was only in one datapoint
      // average weighted by inddens:
      CHECK(d1.expenditure ==
            Approx((1.0 + 2.0 * 2.0 * 2.0) / (1.0 + 2.0 * 2.0)));
    }
  }
}
