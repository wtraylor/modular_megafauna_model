// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Output::CombinedData.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "catch.hpp"
#include "combined_data.h"
#include "dummy_hft.h"
#include "parameters.h"
using namespace Fauna;
using namespace Fauna::Output;

TEST_CASE("Fauna::Output::CombinedData") {
  CombinedData c1, c2;
  CHECK(c1.datapoint_count == 0);

  // create some habitat data
  HabitatData hab1, hab2;
  hab1.available_forage.grass.set_mass(1.0);
  hab1.available_forage.grass.set_fpc(.5);
  hab2.available_forage.grass.set_mass(2.0);
  hab2.available_forage.grass.set_fpc(.5);

  // put habitat data into CombinedData
  c1.habitat_data = hab1;
  c2.habitat_data = hab2;

  // create some herbivore data

  const HftList hfts = create_hfts(2, Parameters());
  REQUIRE(hfts.size() == 2);

  HerbivoreData h1, h2;  // HFT 0
  HerbivoreData h3;      // HFT 1

  h1.inddens = 1.0;
  h2.inddens = 2.0;
  h3.inddens = 3.0;
  h1.expenditure = 1.0;
  h2.expenditure = 2.0;
  h3.expenditure = 3.0;

  // put the herbivore data into CombinedData

  c1.hft_data[hfts[0].get()->name] = h1;
  // note: c2 has no entry for HFT 1
  c1.hft_data[hfts[1].get()->name] = h3;
  c2.hft_data[hfts[0].get()->name] = h2;

  SECTION("merge() with zero datapoint count") {
    c1.datapoint_count = 0;
    c2.datapoint_count = 1;

    c2.merge(c1);

    // c2 should be left unchanged

    CHECK(c2.datapoint_count == 1);
    CHECK(c2.hft_data.size() == 1);
    CHECK(c2.habitat_data.available_forage.grass.get_mass() == 2.0);
  }

  SECTION("merge() with equal datapoint counts") {
    // merge with equal weight
    c1.datapoint_count = c2.datapoint_count = 3;

    c1.merge(c2);

    CHECK(c1.datapoint_count == 6);

    CHECK(c1.habitat_data.available_forage.grass.get_mass() == Approx(1.5));
    // c1 now has data for both HFTs
    REQUIRE(c1.hft_data.size() == 2);
    // in HFT 0, two datapoints are merged
    CHECK(c1.hft_data[hfts[0].get()->name].inddens ==
          Approx((1.0 + 2.0) / 2.0));  // normal average
    CHECK(
        c1.hft_data[hfts[0].get()->name].expenditure ==
        Approx((1.0 * 1.0 + 2.0 * 2.0) / (1.0 + 2.0)));  // weighted by inddens

    // in HFT 1, one datapoint is merged with zero values
    CHECK(c1.hft_data[hfts[1].get()->name].inddens ==
          Approx((0.0 + 3.0) / 2.0));  // normal average
    CHECK(c1.hft_data[hfts[1].get()->name].expenditure ==
          Approx(3.0));  // weighted by inddens, but only one data point
  }

  SECTION("merge() with different data point counts") {
    c1.datapoint_count = 1;
    c2.datapoint_count = 2;

    c1.merge(c2);

    CHECK(c1.datapoint_count == 3);

    CHECK(c1.habitat_data.available_forage.grass.get_mass() ==
          Approx((1.0 * 1.0 + 2.0 * 2.0) / (3.0)));
    // c1 now has data for both HFTs
    REQUIRE(c1.hft_data.size() == 2);

    // in HFT 0, two datapoints are merged
    CHECK(c1.hft_data[hfts[0].get()->name].inddens ==
          Approx((1.0 + 2.0 * 2.0) / 3.0));  // weighted by datapoint_count

    // weighted by inddens*datapoint_count:
    CHECK(c1.hft_data[hfts[0].get()->name].expenditure ==
          Approx((1.0 + 2.0 * 2.0 * 2.0) / (1.0 + 2.0 * 2.0)));

    // in HFT 1, one datapoint is merged with zero values
    CHECK(c1.hft_data[hfts[1].get()->name].inddens ==
          Approx((1.0 * 3.0 + 2.0 * 0.0) /
                 (1.0 + 2.0)));  // weighted by datapoint_count

    // weighted by inddens*datapoint_count, but since c2 has no data for
    // HFT 1, and since expenditure is individual based, c2 is simply not
    // included in the average.
    CHECK(c1.hft_data[hfts[1].get()->name].expenditure == h3.expenditure);
  }

  SECTION("reset()") {
    c1.reset();
    CHECK(c1.datapoint_count == 0);
    CHECK(c1.hft_data.empty());
    CHECK(c1.habitat_data.available_forage.get_mass().sum() == 0.0);
  }
}
