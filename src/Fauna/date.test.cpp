#include "catch.hpp"
#include "date.h"

using namespace Fauna;

TEST_CASE("Fauna::Date", "") {
  CHECK_THROWS(Date(366, 0));
  CHECK_NOTHROW(Date(365, 0));
  CHECK_NOTHROW(Date(0, 0));

  SECTION("next day within one year") {
    for (int year = -3; year <= +3; year++)
      for (int day = 0; day < 365; day++) {
        const Date d1(day, year);
        // If next_day==365, it’s a leap year.
        for (int next_day = 0; next_day < 366; next_day++) {
          const Date d2(next_day, year);
          if (next_day == day + 1)
            CHECK(d1.is_successive(d2));
          else
            CHECK(!d1.is_successive(d2));
        }
      }
  }

  SECTION("next day at year boundary") {
    for (int year = -3; year <= +3; year++)
      for (int next_year = -3; next_year <= +3; next_year++) {
        if (year == next_year)
          continue;  // Within-year succession already checked.
        for (int day = 0; day < 366; day++) {
          const Date d1(day, year);
          for (int next_day = 0; next_day < 366; next_day++) {
            const Date d2(next_day, next_year);
            // Only at the last day, Jan 1st of the directly following
            // year is successive.
            const bool is_successive =
                (next_year == year + 1 && next_day == 0 &&
                 (day == 364 || day == 365));
            CHECK(d1.is_successive(d2) == is_successive);
          }
        }
      }
  }
}
