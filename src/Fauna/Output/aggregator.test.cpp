// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Output::Aggregator.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "aggregator.h"
#include "catch.hpp"

using namespace Fauna;
using namespace Fauna::Output;

TEST_CASE("Fauna::Output::Aggregator", "") {
  Aggregator agg;

  // No data added yet.
  CHECK_THROWS(agg.get_interval());
  CHECK(agg.retrieve().empty());

  static const std::string UNIT1 = "unit1";
  static const std::string UNIT2 = "unit2";
  static const std::string UNIT3 = "unit3";
  static const Date DATE1 = Date(1, 1);
  static const Date DATE2 = Date(10, 1);
  static const Date DATE3 = Date(11, 1);

  agg.add(DATE1, UNIT1, CombinedData());
  CHECK(agg.get_interval().get_first() == DATE1);
  CHECK(agg.get_interval().get_first() == DATE1);

  SECTION("Retrieve first datapoint") {
    const std::vector<Datapoint> v = agg.retrieve();
    CHECK(v.size() == 1);  // one aggregation unit
    CHECK_THROWS(agg.get_interval());
    CHECK(agg.retrieve().empty());

    CHECK(v[0].interval.get_first() == DATE1);
    CHECK(v[0].interval.get_last() == DATE1);
  }

  agg.add(DATE2, UNIT1, CombinedData());

  SECTION("Retrieve one aggregation unit") {
    const std::vector<Datapoint> v = agg.retrieve();
    CHECK(v.size() == 1);  // one aggregation unit
    CHECK_THROWS(agg.get_interval());
    CHECK(agg.retrieve().empty());

    CHECK(v[0].interval.get_first() == DATE1);
    CHECK(v[0].interval.get_last() == DATE2);
  }

  agg.add(DATE1, UNIT2, CombinedData());
  agg.add(DATE2, UNIT2, CombinedData());
  agg.add(DATE3, UNIT2, CombinedData());

  agg.add(DATE1, UNIT3, CombinedData());
  agg.add(DATE2, UNIT3, CombinedData());

  SECTION("Retrieve multiple datapoints") {
    const std::vector<Datapoint> v = agg.retrieve();
    CHECK(v.size() == 3);  // 3 aggregation units
    CHECK_THROWS(agg.get_interval());
    CHECK(agg.retrieve().empty());

    for (const auto& i : v) {
      CHECK(i.interval.get_first() == DATE1);
      CHECK(i.interval.get_last() == DATE3);
      // All intervals are the same.
      for (const auto& j : v) {
        CHECK(i.interval.get_first() == j.interval.get_first());
        CHECK(i.interval.get_last() == j.interval.get_last());
      }
    }
  }
}
