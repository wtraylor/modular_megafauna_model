#include "catch.hpp"
#include "date_interval.h"
#include "parameters.h"

using namespace Fauna;

TEST_CASE("Fauna::DateInterval", "") {
  static const int YEAR = 4;  // arbitrary

  SECTION("OutputInterval::extend") {
    DateInterval interval(Date(0, 0), Date(0, 0));

    REQUIRE(interval.get_first() == Date(0, 0));
    REQUIRE(interval.get_last() == Date(0, 0));

    // Extend to the back
    interval.extend(Date(10, 1));
    CHECK(interval.get_first() == Date(0, 0));
    CHECK(interval.get_last() == Date(10, 1));

    // No changes
    interval.extend(Date(50, 0));
    CHECK(interval.get_first() == Date(0, 0));
    CHECK(interval.get_last() == Date(10, 1));

    // Extend to the front
    interval.extend(Date(120, -1));
    CHECK(interval.get_first() == Date(120, -1));
    CHECK(interval.get_last() == Date(10, 1));
  }

  SECTION("OutputInterval::Annual") {
    for (int day = 0; day < 366; day++)
      for (int year = YEAR; year < YEAR + 3; year++) {
        const DateInterval interval(Date(0, YEAR), Date(day, year));
        const bool is_not_annual = ((day != 364 && day != 365) || year != YEAR);
        CHECK(interval.matches_output_interval(OutputInterval::Annual) ==
              !is_not_annual);
      }
  }

  // TODO Check for other output interval matches
}
