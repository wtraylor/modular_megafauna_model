#include "catch.hpp"
#include "utils.h"

using namespace Fauna;

TEST_CASE("Fauna::average()", "") {
  CHECK_THROWS(average(1.0, 2.0, -1.0, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, -1.0));
  CHECK_THROWS(average(1.0, 2.0, 0.0, 0.0));
  CHECK_THROWS(average(1.0, 2.0, NAN, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, NAN));
  CHECK_THROWS(average(1.0, 2.0, INFINITY, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, INFINITY));
  CHECK(average(1.0, 3.0) == Approx(2.0));
  CHECK(average(1.0, 1.0) == Approx(1.0));
  CHECK(average(-1.0, 1.0) == Approx(0.0));
}

TEST_CASE("Fauna::get_day_of_month()") {
  CHECK_THROWS(get_day_of_month(-1));
  CHECK_THROWS(get_day_of_month(365));
  // Just check some arbitrary dates
  CHECK(get_day_of_month(0) == 0);        // Jan 1st
  CHECK(get_day_of_month(364) == 30);     // Dec. 1st
  CHECK(get_day_of_month(32) == 1);       // Feb. 2nd
  CHECK(get_day_of_month(31 + 28) == 0);  // Mar. 1st
}

TEST_CASE("Fauna::get_random_fraction", "") {
  for (int i = 0; i < 100; i++) {
    const double r = get_random_fraction();
    CHECK(r <= 1.0);
    CHECK(r >= 0.0);
  }
}

TEST_CASE("Fauna::PeriodAverage") {
  CHECK_THROWS(PeriodAverage(-1));
  CHECK_THROWS(PeriodAverage(0));

  const int COUNT = 3;
  PeriodAverage pa(COUNT);

  CHECK_THROWS(pa.get_average());

  const double A = .1;
  const double B = .2;
  const double C = .4;
  const double D = .5;
  const double E = .6;

  pa.add_value(A);
  CHECK(pa.get_average() == A);

  pa.add_value(B);
  CHECK(pa.get_average() == Approx((A + B) / 2.0));

  pa.add_value(C);
  CHECK(pa.get_average() == Approx((A + B + C) / 3.0));

  pa.add_value(D);
  CHECK(pa.get_average() == Approx((B + C + D) / 3.0));

  pa.add_value(E);
  CHECK(pa.get_average() == Approx((C + D + E) / 3.0));
}
